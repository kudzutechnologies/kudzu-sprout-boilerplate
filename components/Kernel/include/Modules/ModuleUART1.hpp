#ifndef KUDZUKERNEL_MODULEUART1_H
#define KUDZUKERNEL_MODULEUART1_H
#include "ModuleUART.hpp"
#include "Utilities/Lockable.hpp"

extern const esp_event_base_t UART1_EVENT_BASE;

/**
 * UART1 implementation of ModuleUART
 */
class _ModuleUART1: public _ModuleUART, public Lockable {
public:

  /**
   * Constructor
   */
  _ModuleUART1();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

};

extern _ModuleUART1 ModuleUART1;

#endif
