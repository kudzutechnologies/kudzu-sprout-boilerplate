#ifndef KUDZUKERNEL_MODULEFUELGAUGE_H
#define KUDZUKERNEL_MODULEFUELGAUGE_H
#include <Module.hpp>

#define SUBCLASS_ID_SAFETY                2
#define SUBCLASS_ID_CHARGE_INHIBIT_CFG    32
#define SUBCLASS_ID_CHARGE                34
#define SUBCLASS_ID_CHARGE_TERMINATION    36
#define SUBCLASS_ID_DATA                  48
#define SUBCLASS_ID_DISCHARGE             49
#define SUBCLASS_ID_MANUFACTURER_DATA     56
#define SUBCLASS_ID_LIFETIME_DATA         59
#define SUBCLASS_ID_LIFETIME_TEMP_SAMPLES 60
#define SUBCLASS_ID_REGISTERS             64
#define SUBCLASS_ID_LIFETIME_RESOLUTION   66
#define SUBCLASS_ID_LED_DISPLAY           67
#define SUBCLASS_ID_POWER                 68
#define SUBCLASS_ID_MANUFACTURER_INFO     58
#define SUBCLASS_ID_IT_CFG                80
#define SUBCLASS_ID_CURRENT_THRESHOLDS    81
#define SUBCLASS_ID_STATE                 82
#define SUBCLASS_ID_R_A0                  88
#define SUBCLASS_ID_R_A0X                 89
#define SUBCLASS_ID_CAL_DATA              104
#define SUBCLASS_ID_CAL_CURRENT           107
#define SUBCLASS_ID_CODES                 112

////////////////////////////////////////////////////////////////////////////////////////
enum ModuleFuelGaugeEvents {
  EVENT_FUELGAUGE_READY,
  EVENT_FUELGAUGE_GET_MEASUREMENT,
  EVENT_FUELGAUGE_TIMED_MEASUREMENT,
};

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleFuelGauge: public Module {
public:

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

protected:

  /**
   * Return the list of configuration options to expose to the user
   * for this module;
   */
  virtual std::vector<ValueDefinition> configOptions();

  /**
   * Called when the user has commited the changes to the module configuration
   */
  virtual void configDidSave();

  /**
   * Initialize NVS with the default value
   */
  virtual void nvsReset(void* nvs);

  /**
   * Initialize setup
   */
  virtual void setup();

  /**
   * Declare a few event handlers
   */
  DECLARE_EVENT_HANDLER(all_events);
  DECLARE_EVENT_HANDLER(sensorhub_events);

  /**
   * Module activation and de-activation functions
   */
  virtual void activate();
  virtual void deactivate();

private:

  bool activating;
  bool devicePresent[4];
  char deviceStatus[4][32];

};

extern _ModuleFuelGauge ModuleFuelGauge;

#endif
