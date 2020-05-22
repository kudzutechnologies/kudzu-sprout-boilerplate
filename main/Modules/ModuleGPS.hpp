#ifndef KUDZUKERNEL_ModuleGPS_H
#define KUDZUKERNEL_ModuleGPS_H
#include <Module.hpp>
#include <queue>
#include "Utilities/WithFSM.hpp"
#include "Utilities/Measurement.hpp"
#include "ModuleNMEAParser.hpp"

/**
 * Forward declaration of the module singleton
 */
class _ModuleGPS;
extern _ModuleGPS ModuleGPS;

///////////////////////////////////////////
// Declaration of events that can be used
///////////////////////////////////////////

/**
 * An enum that defines the events your modules exchanges
 */
enum ModuleGPSEvents {
  EVENT_GPS_FIXED,
  EVENT_GPS_LOST,
  EVENT_GPS_POSITION,
  EVENT_GPS_TIME,

  EVENT_GPS_WRITE_CHUNK = 0x100,
  EVENT_GPS_AGPS_DONE,
};

/**
 * A structure that holds a GPS position
 */
struct gps_event_t {
  float latitude;
  float longitude;
  float altitude;
  float quality;
};


///////////////////////////////////////////
// Declaration of the module
///////////////////////////////////////////

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleGPS: public Module, public WithFSM {
public:

  _ModuleGPS();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

  /**
   * Upload AssistNOW offline geolocation data
   */
  esp_err_t applyAssistNow(const char * payload, const size_t len);

protected:

  ///////////////////////////////
  // Module life-cycle methods
  ///////////////////////////////

  /**
   * (Optional) Implement this method to initialize your module
   */
  virtual void setup();

  ///////////////////////////////
  // Event handlers
  ///////////////////////////////

  /**
   * Declare one or more event handlers
   *
   * These event handlers can either listen for events from this module
   */
  DECLARE_EVENT_HANDLER(gps_events);
  DECLARE_EVENT_HANDLER(nmea_events);
  DECLARE_EVENT_HANDLER(sensorhub_events);

  ///////////////////////////////
  // User interface binding
  ///////////////////////////////

  /**
   * (Optional) Implement this method to return the UI options for this module
   */
  virtual std::vector<ValueDefinition> configOptions();

  /**
   * (Optional) Implement this method if you want to take any action at UI save
   * By default this method will save the non-volatile storage object.
   */
  virtual void configDidSave();

  /**
   * Initialize NVS with the default value
   */
  virtual void nvsReset(void* nvs);

  /**
   * (Optional) Implement this method to handle activation of your module
   */
  virtual void activate();

  /**
   * (Optional) Implement this method to handle de-activation of your module
   */
  virtual void deactivate();

private:

  /**
   * Send data using the UBX protocol
   */
  int ubxWrite(uint8_t msgClass, uint8_t msgId, const uint8_t* data, const size_t data_len);

  void ubxSendTime();

private:

  /**
   * This holds a pointer to the last `sendPostAfter` command
   */
  ModuleTimer_t activeTimer;

  /**
   * SARA_EN GPIO configuration
   */
  gpio_config_t   io_conf;

  /**
   * The currently visible satelites
   */
  bool            v_fix;
  uint8_t         v_siv_gps, v_siv_glonass, v_siv_galileo;
  char            v_sat_in_view[40];

  /**
   * Last known location
   */
  float           fix_lat, fix_lng, fix_alt, fix_cog, fix_speed;

  /**
   * The last GPS measurement
   */
  Measurement     lastMeasurement;

};


#endif
