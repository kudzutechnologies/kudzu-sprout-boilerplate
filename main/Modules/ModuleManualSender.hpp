#ifndef KUDZUKERNEL_ModuleManualSender_H
#define KUDZUKERNEL_ModuleManualSender_H
#include <Module.hpp>
#include "Modules/ModuleSARASockets.hpp"

/**
 * Forward declaration of the module singleton
 */
class _ModuleManualSender;
extern _ModuleManualSender ModuleManualSender;

///////////////////////////////////////////
// Declaration of events that can be used
///////////////////////////////////////////

/**
 * An enum that defines the events your modules exchanges
 */
enum ModuleManualSenderEvents {
};

///////////////////////////////////////////
// Declaration of the module
///////////////////////////////////////////

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleManualSender: public Module {
public:

  _ModuleManualSender();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

private:

  /**
   * Initialize the module
   */
  virtual void setup();

  virtual void activate();

  /**
   * (Optional) Implement this method to return the UI options for this module
   */
  virtual std::vector<ValueDefinition> configOptions();

  virtual void configDidSave();

  ///////////////////////////////
  // Event handlers
  ///////////////////////////////

  /**
   * Declare one or more event handlers
   *
   * These event handlers can either listen for events from this module
   */
  DECLARE_EVENT_HANDLER(all_events);

  ConnectionHandler_t   h_socket;
  char                  txBuffer[1024];
  bool                  sending;
  bool                  failed;

};


#endif
