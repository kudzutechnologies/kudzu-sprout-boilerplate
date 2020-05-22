#ifndef KUDZUKERNEL_WAITGROUPPOOL_HPP
#define KUDZUKERNEL_WAITGROUPPOOL_HPP
#include <vector>
#include "WaitGroup.hpp"
#include "esp_log.h"

template<typename WG, uint8_t SZ>
class WaitGroupPool {
public:

  WaitGroupPool() {
    memset(&__v0001[0], 0, sizeof(WG) * SZ);
  }

  /**
   * Returns a free WaitGroup instance from the pool
   */
  WG* free() {
    for (uint8_t i=0; i<SZ; ++i) {
      if (__v0001[i].isFree()) {
        // Make sure to call the constructors, otherwise the
        // virtual pointers won't be initialized!
        return new(&__v0001[i])WG();
      }
    }

    return NULL;
  }

private:
  WG     __v0001[SZ];
};

#endif
