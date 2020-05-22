#include "Modules/ModuleSensorHub.hpp"
#include "KudzuKernel.hpp"
#include "Sherlock.hpp"
#include "ModuleHPM.hpp"

// Instantiate singleton
_ModuleHPM ModuleHPM;

/**
 * The module name, also used for logging
 */
static const char * TAG = "logic.hpm";

/**
 * @brief      The persistent module configuration
 * @details    Describes the data structure that is persisted in the flash memory
 *             of the chip and can be used to keep configuration (or other kind)
 *             of data that will persist across restarts.
 */
struct HPMNvsConfig {
  int   sample_interval;
  int   flush_interval;
};

/**
 * @brief      Returns the module configuration
 * @details    This function should return a static singleton with the module
 *             configuration that will be used by the kernel during load-time
 *
 * @return     The module configuration.
 */
const ModuleConfig& _ModuleHPM::getModuleConfig() {
  static const ModuleConfig config = {
    .name = TAG,
    .title = "High-Power Controller",
    .category = MODULE_CATEGORY_DEFAULT,
    .nv_size = sizeof(HPMNvsConfig),
    .nv_version = 1,
    .runlevels = {
      RUNLEVEL_EXT_POWER
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
void _ModuleHPM::setup() {

  // Register a local handler to all of the events of the module-wide scope
  EVENT_HANDLER_REGISTER(all_events, ESP_EVENT_ANY_ID);

}

/**
 * @brief      Activate the module
 * @details    Called when the module should be activated. This function is always called
 *             when all the dependent modules are activated first and is going to block
 *             further activation of other modules depending on this until `ackActivate` is called
 */
void _ModuleHPM::activate() {

  // Immediately when started, request to take a sample
  eventPost(EVENT_HPM_SAMPLE, NULL, 0);

  ackActivate();
}

/**
 * @brief      Deactivate the module
 * @details    Called when the module should be deactivated. This function is always called
 *             before the dependent modules are deactivated and is going to block further
 *             deactivation of other modules depending on this until `ackDeactivate` is called
 */
void _ModuleHPM::deactivate() {
  eventTimerStopAll();
  ackDeactivate();
}

///////////////////////////////
// Event handlers
///////////////////////////////

/**
 * Event handler for all events received in the module scope
 */
DEFINE_EVENT_HANDLER(_ModuleHPM::all_events)(esp_event_base_t event_base, int32_t event_id, void *event_data) {
  HPMNvsConfig * conf = (HPMNvsConfig*)nvs();

  switch (event_id) {
  case EVENT_HPM_SAMPLE:

    // Each sensor is waiting for an event from the sensor hub in order
    // to take a sample. The following command dispatches that event
    TRACE_LOGD(TAG, "Sampling sensors");
    ModuleSensorHub.sampleAllSensors();

    // We schedule the flush timeout
    eventPostAfter(
      EVENT_HPM_FLUSH, NULL, 0,
      (conf->flush_interval * 1000) / portTICK_PERIOD_MS
    );
    break;

  case EVENT_HPM_FLUSH:

    // When the flush timeout occurs, we are asking SensorHub to encode
    // the samples collected so far and push them to the sender module, using
    // the currently installed encoder.
    TRACE_LOGD(TAG, "Flushing data");
    ModuleSensorHub.flush();

    // We schedule the next sample timeout
    eventPostAfter(
      EVENT_HPM_SAMPLE, NULL, 0,
      (conf->sample_interval * 1000) / portTICK_PERIOD_MS
    );

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
std::vector<ValueDefinition> _ModuleHPM::configOptions() {
  HPMNvsConfig * conf = (HPMNvsConfig*)nvs();

  // The UI will update the values bound by reference. So there
  // is no need for explicitly committing the changes.

  return {
    { "Sample Interval",    BIND_INT(conf->sample_interval),    WIDGET_NUMBER(),
      "Number of seconds to wait before requesting all sensors to collect data." },
    { "Flush Interval",     BIND_INT(conf->flush_interval),     WIDGET_NUMBER(),
      "Number of seconds to wait for all sensors to provide their data." }
  };
};

/**
 * @brief      Handle the user clicking "save" on the UI
 */
void _ModuleHPM::configDidSave() {
  HPMNvsConfig * conf = (HPMNvsConfig*)nvs();

  // If the user has not modified any field, the `configChange` property
  // will be `false`
  if (configChanged) {
    eventTimerStopAll();
    eventPostAfter(
      EVENT_HPM_FLUSH, NULL, 0,
      (conf->flush_interval * 1000) / portTICK_PERIOD_MS
    );

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
void _ModuleHPM::nvsReset(void* nvs) {
  HPMNvsConfig * conf = (HPMNvsConfig*)nvs;

  conf->sample_interval = 60 * 60;
  conf->flush_interval = 60;
}
