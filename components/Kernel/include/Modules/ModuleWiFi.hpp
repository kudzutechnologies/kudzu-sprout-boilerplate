#ifndef KUDZUKERNEL_MODULEWIFI_H
#define KUDZUKERNEL_MODULEWIFI_H
#include <Module.hpp>
#include "esp_wifi.h"

enum ModuleWifiEvents {
  EVENT_WIFI_READY,
  EVENT_WIFI_DISABLED,

  EVENT_WIFI_AP_START = 0x100,
  EVENT_WIFI_AP_STOP,
  EVENT_WIFI_AP_STACONNECTED,
  EVENT_WIFI_AP_STADISCONNECTED,
  EVENT_WIFI_AP_STA_START,
  EVENT_WIFI_AP_STA_STOP,
  EVENT_WIFI_AP_STA_GOT_IP,
  EVENT_WIFI_AP_STA_DISCONNECTED,
  EVENT_WIFI_AP_STA_TIMEOUT_IP,
  EVENT_WIFI_AP_STA_RETRY_CONNECT
};

/**
 * Maximum number of connect retries before considering the STA unavailable
 */
#define WIFI_MAX_RETRIES  30

/**
 * Forward declarations
 */
extern const esp_event_base_t WIFI_EVENT_BASE;

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleWiFi: public Module {
public:

  /**
   * Return the __v0007 configuration
   */
  virtual const ModuleConfig& getModuleConfig();

  /**
   * Checks if the WiFi __v0007 is connected to an AP
   */
  bool isConnected();

protected:
  virtual std::vector<ValueDefinition> configOptions();
  virtual void configDidSave();
  virtual void nvsReset(void* nvs);
  virtual void setup();
  DECLARE_EVENT_HANDLER(all_events);
  virtual void activate();
  virtual void deactivate();
  virtual esp_err_t handleSystemEvent(system_event_t *event);
private:
  void  __v0002();
  void  __v0001();
  bool  __v0008;
  bool  __v0005;
  bool  __v0004;
  int   __v0003;
  int   __v0006;
};

extern _ModuleWiFi ModuleWiFi;

#endif
