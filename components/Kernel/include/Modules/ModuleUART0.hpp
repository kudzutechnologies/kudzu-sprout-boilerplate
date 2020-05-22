#ifndef KUDZUKERNEL_MODULEUART0_H
#define KUDZUKERNEL_MODULEUART0_H
#include "ModuleUART.hpp"
#include "Utilities/Lockable.hpp"

extern const esp_event_base_t UART0_EVENT_BASE;

/**
 * UART0 implementation of ModuleUART
 */
class _ModuleUART0: public _ModuleUART, public Lockable {
public:

  /**
   * Constructor
   */
  _ModuleUART0();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

};

extern _ModuleUART0 ModuleUART0;

#endif
