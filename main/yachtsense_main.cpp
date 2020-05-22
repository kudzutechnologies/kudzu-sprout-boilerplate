#include "KudzuKernel.hpp"
#include "Module.hpp"

#include "Modules/ModuleFuelGauge.hpp"
#include "Modules/ModuleSensorHub.hpp"
#include "Modules/ModuleGPS.hpp"
#include "Modules/ModuleHPM.hpp"
#include "Modules/ModuleIMU.hpp"
#include "Modules/ModuleLPM.hpp"
#include "Modules/ModuleJSRuntime.hpp"
#include "Modules/ModuleSender.hpp"
#include "Modules/ModuleManualSender.hpp"
#include "Modules/ModuleNMEAParser.hpp"
#include "Modules/ModuleLoRaConcentrator.hpp"
#include "Modules/ModuleLoRaForwarder.hpp"
#include "Utilities/CayenneEncoder.hpp"
#include "Utilities/JSONEncoder.hpp"

CayenneEncoder encoderCayenne;
JSONEncoder encoderJson;

enum TestEvents {
  EVENT_START = 1,
  EVENT_SEND_LORA,
  EVENT_WIFI_SCAN,
  EVENT_WIFI_OFF,
};

class _MeasurementModule: public Module {
public:

  _MeasurementModule(): Module() {

  }

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig() {
    static const ModuleConfig config = {
      .name = "debug.measurement",
      .title = "Measurement Module",
      .category = MODULE_CATEGORY_DEFAULT,
      .nv_size = 0,
      .nv_version = 0,
      .runlevels = {
        RUNLEVEL_EXT_POWER,
        RUNLEVEL_BAT_POWER
      },
      .activate = DEFAULT_ACTIVE,
      .depends = { }
    };

    return config;
  }


  virtual void setup() {
    tcpip_adapter_init();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    EVENT_HANDLER_REGISTER(all_events, ESP_EVENT_ANY_ID);
  }

  virtual void activate() {
    eventPost(EVENT_START, NULL, 0);
    ackActivate();
  }

  EVENT_HANDLER(all_events)(esp_event_base_t event_base, int32_t event_id, void *event_data) {
    switch (event_id) {
    case EVENT_START:
      eventPostAfter(EVENT_WIFI_SCAN, NULL, 0, 10000 / portTICK_PERIOD_MS);
      break;

    case EVENT_WIFI_SCAN:
      {
        esp_wifi_set_mode(WIFI_MODE_NULL);
        esp_wifi_start();

        wifi_country_t c = {
          .cc = "GR",
          .schan=1,
          .nchan=13,
          .max_tx_power = 20,
          .policy=WIFI_COUNTRY_POLICY_AUTO
        };
        esp_wifi_set_country(&c);

        wifi_scan_config_t scfg;
        memset(&scfg, 0, sizeof(scfg));
        scfg.scan_type = WIFI_SCAN_TYPE_PASSIVE;
        scfg.scan_time.passive = 90;
        esp_wifi_scan_start(&scfg, false);
      }
      eventPostAfter(EVENT_WIFI_OFF, NULL, 0, 10000 / portTICK_PERIOD_MS);
      break;

    case EVENT_WIFI_OFF:
      {
        esp_wifi_set_mode(WIFI_MODE_NULL);
        esp_wifi_stop();
      }
      eventPostAfter(EVENT_SEND_LORA, NULL, 0, 10000 / portTICK_PERIOD_MS);
      break;

    case EVENT_SEND_LORA:
      ModuleSender.sendData("HELLO", 5);
      eventPostAfter(EVENT_WIFI_SCAN, NULL, 0, 30000 / portTICK_PERIOD_MS);
      break;

    }
  }

};

_MeasurementModule MeasurementModule;

/**
 * Application entry point
 */
extern "C" void app_main()
{

  // // Install the encoders we support
  ModuleSensorHub.addEncoder(&encoderCayenne);
  ModuleSensorHub.addEncoder(&encoderJson);

  // Start the kernel with the modules we wish to load
  // Note that the order is not important
  Kernel.start({
    // &ModuleFuelGauge,
    // &ModuleIMU,
#ifdef CONFIG_CONSOLE_UART_NONE
    // We cannot use UART with GPS, so UART must be disabled from
    // SDK config, before enabling GPS
    &ModuleGPS,
    &ModuleNMEAParser,
#else
#warning "Not enabling GPS because UART0 is used for ESP32 logs"
#endif
    // &ModuleLPM,
    // &ModuleHPM,
    // &MeasurementModule
    &ModuleJSRuntime,
    // &ModuleManualSender,
    // &ModuleLoRaConcentrator,
    // &ModuleLoRaForwarder,
  });
}
