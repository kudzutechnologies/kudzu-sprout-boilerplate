#ifndef KUDZUKERNEL_WITHNVS_H
#define KUDZUKERNEL_WITHNVS_H
#include "esp_event.h"

/**
 * The `WithNVS` class provides the base abstraction for segmented access to
 * the device's non-volatile storage.
 */
class WithNVS {
public:

  /**
   * Constructor of the NVS class
   */
  WithNVS();

  /**
   * Return the size of the non-volatile storage required
   */
  virtual size_t nvsSize() = 0;

  /**
   * Return the namespace of the local NVS storage
   */
  virtual const char * nvsNS() = 0;

  /**
   * Return the version of the NVS storage
   */
  virtual uint32_t nvsVersion() = 0;

  /**
   * Initialize NVS with the default value
   */
  virtual void nvsReset(void* nvs);

  /**
   * Returns a pointer to the non-volatile storage structure. This function
   * is guranteed to return the same pointer throughout the execution of the
   * program.
   */
  void * nvs();

  /**
   * Save the changes to the non-volatile storage pointer for this module
   */
  void nvsSave();

  /**
   * Re-load the the NVS storage to the nvs pointer
   */
  void nvsLoad();

private:
  void * __v0001;
};

#endif
