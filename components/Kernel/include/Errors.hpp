#ifndef KUDZUKERNEL_ERRORS_H
#define KUDZUKERNEL_ERRORS_H
#include <stdint.h>

/**
 * An enum of possible run-time errors that can occur
 */
enum errid_t {
  E_NONE = 0,
  E_OUT_OF_MEMORY = 1,
  E_TOO_BIG,
  E_QUEUE_FULL,
  E_UNEXPECTED_CODE_BRANCH,
  E_UNINITIALIZED,
  E_PARAM_ERROR,
  E_INTENTIONAL,
  E_HARDWARE_ERROR,

  // Base offset for ESP errors
  E_ESP_ERROR     = 0x10000000
};

/**
 * Returns a name for the given error
 */
const char * errstr(const uint32_t err);

#endif
