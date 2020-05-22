#include "Modules/ModuleSensorHub.hpp"
#include "KudzuKernel.hpp"
#include "Sherlock.hpp"
#include "ModuleLPM.hpp"

// Instantiate singleton
_ModuleLPM ModuleLPM;

/**
 * The module name, also used for logging
 */
static const char * TAG = "logic.lpm";

/**
 * @brief      The persistent module configuration
 * @details    Describes the data structure that is persisted in the flash memory
 *             of the chip and can be used to keep configuration (or other kind)
 *             of data that will persist across restarts.
 */
struct LPMNvsConfig {
  int   flush_interval;
};

/**
 * @brief      Returns the module configuration
 * @details    This function should return a static singleton with the module
 *             configuration that will be used by the kernel during load-time
 *
 * @return     The module configuration.
 */
const ModuleConfig& _ModuleLPM::getModuleConfig() {
  static const ModuleConfig config = {
    .name = TAG,
    .title = "Low-Power Controller",
    .category = MODULE_CATEGORY_DEFAULT,
    .nv_size = sizeof(LPMNvsConfig),
    .nv_version = 1,
    .runlevels = {
      RUNLEVEL_BAT_POWER
    },
    .activate = USER_ACTIVATE | DEFAULT_ACTIVE,
    .depends = { }
  };
  return config;
}

///////////////////////////////
// Module life-cycle methods
///////////////////////////////

/**
 * @brief      Initialize the module
 * @details    Called when the module is installed in the kernel, before the
 *             system has started booting yet. You can use this method to register
 *             your event handlers or perform first-time initializations of your module
 */
void _ModuleLPM::setup() {

  // Register a local handler to all of the events of the module-wide scope
  EVENT_HANDLER_REGISTER(all_events, ESP_EVENT_ANY_ID);

  // Register a handler that receives all events broadcasted in the SensorHub context
  EVENT_HANDLER_REGISTER_ON(ModuleSensorHub, sensorhub_events, ESP_EVENT_ANY_ID);

}

/**
 * @brief      Activate the module
 * @details    Called when the module should be activated. This function is always called
 *             when all the dependent modules are activated first and is going to block
 *             further activation of other modules depending on this until `ackActivate` is called
 */
void _ModuleLPM::activate() {

  // Immediately when activated, we are placing a request to collect samples
  // from all the sensors.
  eventPost(EVENT_LPM_SAMPLE, NULL, 0);

  // We are also marking the module as busy (this will prevent the device from
  // going to deep sleep)
  moduleSetBusy();

  ackActivate();
}

/**
 * @brief      Deactivate the module
 * @details    Called when the module should be deactivated. This function is always called
 *             before the dependent modules are deactivated and is going to block further
 *             deactivation of other modules depending on this until `ackDeactivate` is called
 */
void _ModuleLPM::deactivate() {
  ackDeactivate();
}

///////////////////////////////
// Event handlers
///////////////////////////////

/**
 * Event handler for all events received in the module scope
 */
DEFINE_EVENT_HANDLER(_ModuleLPM::all_events)(esp_event_base_t event_base, int32_t event_id, void *event_data) {
  LPMNvsConfig * conf = (LPMNvsConfig*)nvs();

  switch (event_id) {
  case EVENT_LPM_SAMPLE:

    // Each sensor is waiting for an event from the sensor hub in order
    // to take a sample. The following command dispatches that event
    TRACE_LOGD(TAG, "Sampling sensors");
    ModuleSensorHub.sampleAllSensors();

    // We schedule the flush timeout
    eventPostAfter(
      EVENT_LPM_FLUSH, NULL, 0,
      (conf->flush_interval * 1000) / portTICK_PERIOD_MS
    );
    break;

  case EVENT_LPM_FLUSH:

    // When the flush timeout occurs, we are asking SensorHub to encode
    // the samples collected so far and push them to the sender module, using
    // the currently installed encoder.
    TRACE_LOGD(TAG, "Flushing data");
    ModuleSensorHub.flush();

    // (Wait for SensorHub to indicate that the flushing process has completed)

    break;

  }
};

/**
 * Event handler for all events received in the SensorHub scope
 */
DEFINE_EVENT_HANDLER(_ModuleLPM::sensorhub_events)(esp_event_base_t event_base, int32_t event_id, void *event_data) {
  switch (event_id) {
  case EVENT_SENSORHUB_FLUSH_COMPLETED:

    // When flush is completed, we can safely go back to sleep.
    TRACE_LOGI(TAG, "Flushing completed");

    // Setting the module to idle, we indicate that this module should no longer
    // block the device transition to deep sleep. If there are no other busy
    // modules the following command will initiate a system shutdown.
    moduleSetIdle();

    break;
  }
};

///////////////////////////////
// User interface binding
///////////////////////////////

/**
 * @brief      Get the module configuration options
 *
 * @return     Returns a list of UI widgets to display
 */
std::vector<ValueDefinition> _ModuleLPM::configOptions() {
  LPMNvsConfig * conf = (LPMNvsConfig*)nvs();

  // The UI will update the values bound by reference. So there
  // is no need for explicitly committing the changes.

  return {
    { "Flush Interval",     BIND_INT(conf->flush_interval),     WIDGET_NUMBER(),
      "Number of seconds to wait for all sensors to provide their data." }
  };
};

/**
 * @brief      Handle the user clicking "save" on the UI
 */
void _ModuleLPM::configDidSave() {
  // If the user has not modified any field, the `configChange` property
  // will be `false`
  if (configChanged) {
    nvsSave();
  }
}

/**
 * @brief      Provide defaults to the persistent configuration
 * @details    This function is called when the NVS storage is new, or if the
 *             persisted version is not the same as the `nv_version` field in
 *             the module configuration.
 *
 * @param      nvs   A pointer to the persisted NVS structure
 */
void _ModuleLPM::nvsReset(void* nvs) {
  LPMNvsConfig * conf = (LPMNvsConfig*)nvs;

  conf->flush_interval = 10;
}
