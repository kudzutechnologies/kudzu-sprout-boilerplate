#include "ModuleManualSender.hpp"
#include "Modules/ModuleSender.hpp"
#include "ModuleManager.hpp"
#include "KudzuKernel.hpp"
#include "Sherlock.hpp"
#include "esp_log.h"

/**
 * Instantiate singleton
 */
_ModuleManualSender ModuleManualSender;

/**
 * Module configuration
 */
static const ModuleConfig config = {
  .name = "sender.manual",
  .title = "Manual Sender",
  .category = MODULE_CATEGORY_DEFAULT,
  .nv_size = 0,
  .nv_version = 0,
  .runlevels = {
    RUNLEVEL_EXT_POWER,
    RUNLEVEL_BAT_POWER,
    RUNLEVEL_TESTING
  },
  .activate = DEFAULT_ACTIVE,
  .depends = { }
};

/**
 * Configuration forwarding
 */
static const char * TAG = config.name;
const ModuleConfig& _ModuleManualSender::getModuleConfig() { return config; }

_ModuleManualSender::_ModuleManualSender() : Module(), h_socket(0), sending(false), failed(false)
{}

/**
 * UI Configuration Options
 */
std::vector<ValueDefinition> _ModuleManualSender::configOptions() {
  return {
    { "Send Data", BIND_FIXED_STRING(txBuffer, 1024), WIDGET_TEXT(),
      "Enter a string and press Save. The data will be sent using the Sender module that will try LoRa and SARA" },
    { "Sending", BIND_BOOL(sending), WIDGET_LED(WLED_GREEN) },
    { "Failed", BIND_BOOL(failed), WIDGET_LED(WLED_RED) }
  };
};

/**
 * Every time there is a configuration change, consider it a request to send data
 */
void _ModuleManualSender::configDidSave() {
  if (configChanged) {
    TRACE_LOGI(TAG, "Sending '%s'", txBuffer);
    sending = true;
    failed = false;
    ModuleSender.sendData(txBuffer, strlen(txBuffer));
  }
}

/**
 * Initialize the module
 */
void _ModuleManualSender::setup() {
  EVENT_HANDLER_REGISTER( all_events, ESP_EVENT_ANY_ID);
  EVENT_HANDLER_REGISTER_ON( ModuleSender, all_events, ESP_EVENT_ANY_ID);
}

/**
 * Start memory probing when activated
 */
void _ModuleManualSender::activate() {
  ackActivate();
}

/**
 * Define the event handler(s) of the module
 */
DEFINE_EVENT_HANDLER(_ModuleManualSender::all_events)(esp_event_base_t event_base, int32_t event_id, void *event_data) {
  switch (event_id) {
  case EVENT_SENDER_SEND_DONE:
    failed = false;
    sending = false;
    break;

  case EVENT_SENDER_SEND_NO_CARRIER:
    failed = true;
    sending = false;
    break;
  }
}
