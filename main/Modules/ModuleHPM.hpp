#ifndef KUDZUKERNEL_ModuleHPM_H
#define KUDZUKERNEL_ModuleHPM_H
#include <Module.hpp>

/**
 * Forward declaration of the module singleton
 */
class _ModuleHPM;
extern _ModuleHPM ModuleHPM;

///////////////////////////////////////////
// Declaration of events that can be used
///////////////////////////////////////////

/**
 * An enum that defines the events this module broadcasts
 */
enum ModuleHPMEvents {
  EVENT_HPM_SAMPLE,
  EVENT_HPM_FLUSH,
};

///////////////////////////////////////////
// Declaration of the module
///////////////////////////////////////////

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleHPM: public Module {
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
