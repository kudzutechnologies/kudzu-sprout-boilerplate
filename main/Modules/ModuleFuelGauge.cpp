#include <cstdio>

#include "ModuleFuelGauge.hpp"
#include "Modules/ModuleI2CExpander.hpp"
#include "Modules/ModuleLoRa.hpp"
#include "Modules/ModuleSensorHub.hpp"

#include "ModuleManager.hpp"
#include "KudzuKernel.hpp"
#include "Sherlock.hpp"

#include "Peripherals/BQ34Z100G1.hpp"
#include "Utilities/SensorConfig.hpp"

#include <Pinout.hpp>

#define I2C_CHANNEL_A 3
#define I2C_CHANNEL_B 2
#define I2C_CHANNEL_C 1
#define I2C_CHANNEL_D 0
#define BQ34Z100 0x55
#define PAGE_SIZE 32
#define TI_DEVICE_TYPE 0x100

/**
 * Module Singleton
 */
_ModuleFuelGauge ModuleFuelGauge;

/**
 * The FuelGauge sensor configuration
 */
static SensorConfig sensor("fg");

/**
 * NVS configuration
 */
struct FuelGaugeNvsConfig {
	bool activate[3];
};

/**
 * Module configuration
 */
static const ModuleConfig config = {
	.name = "drv.fuelgauge",
	.title = "FuelGauge Driver",
  .category = MODULE_CATEGORY_SENSOR,
	.nv_size = sizeof(FuelGaugeNvsConfig),
	.nv_version = 3,
	.runlevels = {
		RUNLEVEL_EXT_POWER,
		RUNLEVEL_BAT_POWER
	},
	.activate = USER_ACTIVATE | DEFAULT_ACTIVE,
	.depends = {
		&ModuleI2CExpander
	}
};

/**
 * Configuration forwarding
 */
static const char *TAG = config.name;
const ModuleConfig &_ModuleFuelGauge::getModuleConfig() { return config; }

static const TickType_t TIMER_FUEL_GAUGE_MEASURE_PERIOD = 10000 / portTICK_PERIOD_MS;

/**
 * Event handler for network events
 */
DEFINE_EVENT_HANDLER(_ModuleFuelGauge::all_events)
(esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	// batteryData_t batteryData;
	TRACE_LOGD(TAG, "event='%s', id='%d'", event_base, event_id);
	switch (event_id)
	{

	// Called by the period timer to collect a sample
	case EVENT_FUELGAUGE_TIMED_MEASUREMENT:

		// Re-schedule the timer
		// (We need this event to update the state of the connected sensors)
		eventPostAfter(EVENT_FUELGAUGE_TIMED_MEASUREMENT, NULL, 0, TIMER_FUEL_GAUGE_MEASURE_PERIOD);

	// Called when we must collect a sample
	case EVENT_FUELGAUGE_GET_MEASUREMENT:

		TRACE_LOGD(TAG, "Sampling FuelGauge channels");
		{
			bool found = false;
			uint16_t v_dt, v_cap, v_voltage, v_current, v_temp, v_soh;
			uint8_t v_soc;
			esp_err_t err;

			// Scan all channels for a FuelGauge sensor
			for(int i =0 ; i< 4; i++){

				// Swap channel IDs in order to map like so:
				// Device A (3) = 0
				// Device B (2) = 1
				// Device C (1) = 2
				// Device D (0) = 3
				uint8_t id = 3 - i;

				// Set I2C Expander Channel
				ModuleI2CExpander.setChannel(1 + i);

				// Try to read the device type
				err = bq34z100g1_deviceType(ModuleI2CExpander, &v_dt);
				if ((err == ESP_OK) && (v_dt == TI_DEVICE_TYPE)) {
					found = true;

					if (!devicePresent[id]) {
						devicePresent[id] = true;

						// Unseal the device
						ESP_ERROR_OOPS(bq34z100g1_unseal(ModuleI2CExpander, 0x36720414));

						// Configure pack
						bq34z100g1_pack_config cfg;
						ESP_ERROR_OOPS(bq34z100g1_getPackConfig(ModuleI2CExpander, &cfg));
						cfg.voltsel = 1; // Use external voltage divider
						ESP_ERROR_OOPS(bq34z100g1_setPackConfig(ModuleI2CExpander, &cfg));

						// Configure voltage divider
						// bq34z100g1_calib_data data;
						ESP_ERROR_OOPS(bq34z100g1_setVoltageDivider(ModuleI2CExpander, 19239));

						ESP_ERROR_OOPS(bq34z100g1_reset(ModuleI2CExpander));
					}

					ESP_ERROR_OOPS(bq34z100g1_stateOfCharge(ModuleI2CExpander, &v_soc));
					ESP_ERROR_OOPS(bq34z100g1_remainingCapacity(ModuleI2CExpander, &v_cap));
					ESP_ERROR_OOPS(bq34z100g1_voltage(ModuleI2CExpander, &v_voltage));
					ESP_ERROR_OOPS(bq34z100g1_averageCurrent(ModuleI2CExpander, &v_current));
					ESP_ERROR_OOPS(bq34z100g1_temperature(ModuleI2CExpander, &v_temp));
					ESP_ERROR_OOPS(bq34z100g1_stateOfHealth(ModuleI2CExpander, &v_soh));

					TRACE_LOGI(TAG, "Found FuelGauge sensor=%d {SOC=%d, Capacity=%d, Volt=%d, Amp=%d, Temp=%d, SOH=%d}",
						id, v_soc, v_cap, v_voltage, v_current, v_temp, v_soh
					);

					float v = (float)v_voltage / 1000;
					float a = (float)v_current / 1000;
					snprintf(deviceStatus[id], 32, "\xe2\x9c\x85 %d%% (%.2f V, %0.2f A)", v_soc, v, a);

					bq34z100g1_calib_data cd;
					bq34z100g1_getCalibData(ModuleI2CExpander, &cd);
					TRACE_LOGD(TAG, "--[ Calibration Data ]--");
					TRACE_LOGD(TAG, "        cc_gain = %.2f", cd.cc_gain);
					TRACE_LOGD(TAG, "       cc_delta = %.2f", cd.cc_delta);
					TRACE_LOGD(TAG, "      cc_offset = %d", cd.cc_offset);
					TRACE_LOGD(TAG, "   board_offset = %d", cd.board_offset);
					TRACE_LOGD(TAG, "int_temp_offset = %d", cd.int_temp_offset);
					TRACE_LOGD(TAG, "ext_temp_offset = %d", cd.ext_temp_offset);
					TRACE_LOGD(TAG, "voltage_divider = %d", cd.voltage_divider);

					// If this is a request to get a measurement, send the computed
					// values to SensorHub
					if (event_id == EVENT_FUELGAUGE_GET_MEASUREMENT) {

						// Add FuelGauge measurements as fuelgauge/X where X is the channel
						auto g = sensor.group(id);
						ModuleSensorHub.addMeasurement(g, {
							{ "soc", v_soc },
							{ "cap", v_cap },
							{ "voltage", v_voltage },
							{ "current", v_current },
							{ "temp", v_temp },
							{ "soh", v_soh },
						});
					}

				} else {
					snprintf(deviceStatus[id], 32, "\xe2\xac\x9c No Device");
				}
			}

			if (!found) {
				TRACE_LOGW(TAG, "No sensors found connected");
			}
		}
		break;

	default:
		TRACE_LOGD(TAG, "Default");
		break;
	}
}

/**
 * Event handler for sensorhub events
 */
DEFINE_EVENT_HANDLER(_ModuleFuelGauge::sensorhub_events)(esp_event_base_t event_base, int32_t event_id, void *event_data) {
  switch (event_id) {
  case EVENT_SENSORHUB_SAMPLE:
    TRACE_LOGD(TAG, "Responding to SensorHub sample request");
    eventPost(EVENT_FUELGAUGE_GET_MEASUREMENT, NULL, 0);
    break;
  };
}

/**
 * Return the list of configuration options to expose to the user
 * for this module;
 */
std::vector<ValueDefinition> _ModuleFuelGauge::configOptions()
{
	// FuelGaugeNvsConfig *conf = (FuelGaugeNvsConfig *)nvs();

	return {
		{ "Slot A", BIND_FIXED_STRING(deviceStatus[0], 32), WIDGET_LABEL() },
		{ "Slot B", BIND_FIXED_STRING(deviceStatus[1], 32), WIDGET_LABEL() },
		{ "Slot C", BIND_FIXED_STRING(deviceStatus[2], 32), WIDGET_LABEL() },
	};
};

/**
 * Called when the user has committed the changes to the module configuration
 */
void _ModuleFuelGauge::configDidSave()
{
	nvsSave();
}

/**
 * Initialize NVS with the default value
 */
void _ModuleFuelGauge::nvsReset(void *nvs)
{
	FuelGaugeNvsConfig *conf = (FuelGaugeNvsConfig *)nvs;
	memset(conf, 0, sizeof(FuelGaugeNvsConfig));
}

/**
 * Entry level stack initialization
 */
void _ModuleFuelGauge::setup()
{
	TRACE_LOGD(TAG, "_ModuleFuelGauge::setup()");
	EVENT_HANDLER_REGISTER(all_events, ESP_EVENT_ANY_ID);
	EVENT_HANDLER_REGISTER_ON(ModuleSensorHub, sensorhub_events, ESP_EVENT_ANY_ID);

	// Reset status
	for (int i=0; i<4; ++i) {
		devicePresent[i] = false;
		memset(deviceStatus[i], 0, 32);
		snprintf(deviceStatus[i], 32, "\xe2\xac\x9c No Device");
	}
}

/**
 * Module activation and de-activation functions
 */
void _ModuleFuelGauge::activate()
{
	TRACE_LOGD(TAG, "_ModuleFuelGauge::activate()");
	activating = true;

	// Start get measurement poller and also take an immediate measurement
	eventPostAfter(EVENT_FUELGAUGE_TIMED_MEASUREMENT, NULL, 0, TIMER_FUEL_GAUGE_MEASURE_PERIOD);
	eventPost(EVENT_FUELGAUGE_GET_MEASUREMENT, NULL, 0);

	for (int i=0; i<4; ++i) {
		devicePresent[i] = false;
		memset(deviceStatus[i], 0, 32);
		snprintf(deviceStatus[i], 32, "\xe2\xac\x9c No Device");
	}

	ackActivate();
}

void _ModuleFuelGauge::deactivate()
{
	TRACE_LOGD(TAG, "_ModuleFuelGauge::deactivate()");

	eventTimerStopAll();
	ackDeactivate();
}

