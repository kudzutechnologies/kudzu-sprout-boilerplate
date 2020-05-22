#ifndef KUDZUKERNEL_ModuleLPM_H
#define KUDZUKERNEL_ModuleLPM_H
#include <Module.hpp>

/**
 * Forward declaration of the module singleton
 */
class _ModuleLPM;
extern _ModuleLPM ModuleLPM;

///////////////////////////////////////////
// Declaration of events that can be used
///////////////////////////////////////////

/**
 * An enum that defines the events this module broadcasts
 */
enum ModuleLPMEvents {
  EVENT_LPM_SAMPLE,
  EVENT_LPM_FLUSH
};

///////////////////////////////////////////
// Declaration of the module
///////////////////////////////////////////

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleLPM: public Module {
public:

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

protected:

  ///////////////////////////////
  // Module life-cycle methods
  ///////////////////////////////

  /**
   * Initialize the module
   */
  virtual void setup();

  ///////////////////////////////
  // Event handlers
  ///////////////////////////////

  /**
   * Handler for all module-level events
   */
  DECLARE_EVENT_HANDLER(all_events);
  DECLARE_EVENT_HANDLER(sensorhub_events);

  ///////////////////////////////
  // User interface binding
  ///////////////////////////////

  /**
   * Return the UI configuration options
   */
  virtual std::vector<ValueDefinition> configOptions();

  /**
   * Handle UI changes commit event
   */
  virtual void configDidSave();

  /**
   * Initialize NVS with the default value
   */
  virtual void nvsReset(void *nvs);

  /**
   * (Optional) Implement this method to handle activation of your module
   */
  virtual void activate();

  /**
   * (Optional) Implement this method to handle de-activation of your module
   */
  virtual void deactivate();
};


#endif
