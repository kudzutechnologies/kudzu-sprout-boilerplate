#ifndef KUDZUKERNEL_MODULEVBUS_H
#define KUDZUKERNEL_MODULEVBUS_H
#include <Module.hpp>
#include <driver/gpio.h>
#include <Pinout.hpp>

/**
 * Events broadcasted or used in the ModuleVBus
 */
enum ModuleVBusEvents {
  EVENT_VBUS_RAMPUP_DONE
};

/**
 * VBus module is responsible for activating or de-activating the VBus on the device
 */
class _ModuleVBus: public Module {
public:

  _ModuleVBus();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

protected:
  virtual void setup();
  DECLARE_EVENT_HANDLER(all_events);
  virtual void activate();
  virtual void deactivate();
private:
  gpio_config_t   __v0001;
};

extern _ModuleVBus ModuleVBus;

#endif
