#include "ModuleIMU.hpp"
#include "Modules/ModuleSensorHub.hpp"
#include "ModuleManager.hpp"
#include "Utilities/SensorConfig.hpp"
#include "KudzuKernel.hpp"
#include "Sherlock.hpp"
#include "Pinout.hpp"
#include "esp_system.h"
#include "esp_log.h"

#include "mpu/math.hpp"
#include "mpu/types.hpp"

/**
 * Module Singleton
 */
_ModuleIMU ModuleIMU;

/**
 * How frequently to re-try an IMU connection
 */
static const TickType_t RETRY_INTERVAL = 5000 / portTICK_PERIOD_MS;

/**
 * The GPS sensor configuration
 */
static SensorConfig sensor("imu");

/**
 * NVS configuration
 */
struct IMUNvsConfig {
  int sensitivity;
};

static constexpr uint32_t MPU_SPI_CLOCK_SPEED = 10000;  // up to 1MHz for all registers, and 20MHz for sensor data registers only

/**
 * Module configuration
 */
static const ModuleConfig config = {
  .name = "drv.imu",
  .title = "IMU Driver",
  .category = MODULE_CATEGORY_SENSOR,
  .nv_size = sizeof(IMUNvsConfig),
  .nv_version = 1,
  .runlevels = {
    RUNLEVEL_EXT_POWER,
    RUNLEVEL_BAT_POWER
  },
  .activate = USER_ACTIVATE | DEFAULT_INACTIVE,
  .depends = {}
};

/**
 * Configuration forwarding
 */
static const char * TAG = config.name;
const ModuleConfig& _ModuleIMU::getModuleConfig() { return config; }

_ModuleIMU::_ModuleIMU()
  : Module(), mpu_spi_handle(), MPU()
{ }

/**
 * Event handler for network events
 */
DEFINE_EVENT_HANDLER(_ModuleIMU::all_events)(esp_event_base_t event_base, int32_t event_id, void *event_data) {
  TRACE_LOGD(TAG, "event='%s', id='%d'", event_base, event_id);
  esp_err_t err;
  mpud::raw_axes_t accelRaw;   // x, y, z axes as int16
  mpud::raw_axes_t gyroRaw;    // x, y, z axes as int16
  mpud::float_axes_t accelG;   // accel axes in (g) gravity format
  mpud::float_axes_t gyroDPS;  // gyro axes in (DPS) º/s format

  switch (event_id) {
  case EVENT_IMU_CHECK_STARTUP:
    TRACE_LOGD(TAG, "Checking IMU");
    err = MPU.testConnection();
    if (err) {
      TRACE_LOGE(TAG, "Failed to connect to the MPU, error=%#X", err);
      eventPostAfter(EVENT_IMU_RETRY_CONNECTION, NULL, 0, RETRY_INTERVAL);
      break;
    }

    // (Intentionally not breaking here)

  case EVENT_IMU_INITIALIZE:
    TRACE_LOGD(TAG, "Initializing");
    err = MPU.initialize();
    if (err) {
      TRACE_LOGE(TAG, "Failed to initialize MPU, error=%#X", err);
      eventPostAfter(EVENT_IMU_RETRY_CONNECTION, NULL, 0, RETRY_INTERVAL);
      break;
    }

    eventTimerStopAll();
    TRACE_LOGI(TAG, "MPU connected");
    break;

  case EVENT_IMU_RETRY_CONNECTION:
    TRACE_LOGD(TAG, "Checking IMU connection");

    // Check if we wan talk to IMU
    err = MPU.testConnection();
    if (err) {
      TRACE_LOGW(TAG, "IMU is still not responding error=%#X", err);
      eventPostAfter(EVENT_IMU_RETRY_CONNECTION, NULL, 0, RETRY_INTERVAL);
      break;
    }

    // The MPU is responding, initialize now
    eventTimerStopAll();
    eventPost(EVENT_IMU_INITIALIZE, NULL, 0);
    break;

  case EVENT_IMU_READOUT:
    // Read
    MPU.acceleration(&accelRaw);  // fetch raw data from the registers
    MPU.rotation(&gyroRaw);       // fetch raw data from the registers
    // MPU.motion(&accelRaw, &gyroRaw);  // read both in one shot
    // Convert
    accelG = mpud::accelGravity(accelRaw, mpud::ACCEL_FS_4G);
    gyroDPS = mpud::gyroDegPerSec(gyroRaw, mpud::GYRO_FS_500DPS);
    // Debug
    TRACE_LOGI(TAG, "accel: [%+6.2f %+6.2f %+6.2f ] (G) \t", accelG.x, accelG.y, accelG.z);
    TRACE_LOGI(TAG, "gyro: [%+7.2f %+7.2f %+7.2f ] (º/s)\n", gyroDPS[0], gyroDPS[1], gyroDPS[2]);

    ModuleSensorHub.addMeasurement(sensor, {
      { "ax",  accelG.x },
      { "ay",  accelG.y },
      { "az",  accelG.z },
      { "gx",  gyroDPS[0] },
      { "gy",  gyroDPS[1] },
      { "gz",  gyroDPS[2] },
    });
    break;
  }
}

/**
 * Event handler for sensorhub events
 */
DEFINE_EVENT_HANDLER(_ModuleIMU::sensorhub_events)(esp_event_base_t event_base, int32_t event_id, void *event_data) {
  switch (event_id) {
  case EVENT_SENSORHUB_SAMPLE:
    TRACE_LOGD(TAG, "Responding to SensorHub sample request");
    eventPost(EVENT_IMU_READOUT, NULL, 0);
    break;
  };
}

/**
 * Entry level stack initialization
 */
void _ModuleIMU::setup() {
  EVENT_HANDLER_REGISTER(all_events, ESP_EVENT_ANY_ID);
  EVENT_HANDLER_REGISTER_ON(ModuleSensorHub, sensorhub_events, ESP_EVENT_ANY_ID);
}

/**
 * Module activation and de-activation functions
 */
void _ModuleIMU::activate() {
  // IMUNvsConfig * conf = (IMUNvsConfig*)this->nvs();

  // Initialize SPI on HSPI host through SPIbus interface:
  vspi.begin(PIN_E_MOSI, PIN_E_MISO, PIN_E_SCK);
  vspi.addDevice(0, MPU_SPI_CLOCK_SPEED, PIN_EN_IMU, &mpu_spi_handle);

  MPU.setBus(vspi);  // set bus port
  MPU.setAddr(mpu_spi_handle);  // set spi_device_handle, always needed!

  // Perform the start-up checkup
  eventPost(EVENT_IMU_CHECK_STARTUP, NULL, 0);
  ackActivate();
}

void _ModuleIMU::deactivate() {
  TRACE_LOGI(TAG, "Stopping IMU");

  // Make sure there are no lingering timers that could bring the module
  // into an invalid state when they fire!
  eventTimerStopAll();

  // Put chip to sleep
  MPU.setSleep(true);

  vspi.removeDevice(mpu_spi_handle);
  vspi.close();

  ackDeactivate();
}

