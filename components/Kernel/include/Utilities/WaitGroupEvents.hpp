#ifndef KUDZUKERNEL_WAITGROUP_EVENTS
#define KUDZUKERNEL_WAITGROUP_EVENTS
#include <functional>
#include "WaitGroup.hpp"
#include "Sherlock.hpp"

typedef std::function<bool(esp_event_base_t, int event_id, void *event_data)> waitEventHandler;

/**
 * Event loop - based extensions to the WaitGroup
 */
class EWaitGroup: public WaitGroup {
public:

  void waitForEventFromModuleFn(Module * module, waitEventHandler h, size_t event_size);
  void waitForEventFromModule(Module * module, int32_t event_id, size_t event_size);
  void waitForEvent(esp_event_base_t event_base, int32_t event_id, size_t event_size);
  void waitForGlobalEvent(esp_event_base_t event_base, int32_t event_id, size_t event_size);

  void andPostEventToModule(Module * module, int32_t event_id);
  void andPostEvent(esp_event_base_t event_base, int32_t event_id);
  void andPostGlobalEvent(esp_event_base_t event_base, int32_t event_id);

  // void andWait();
  // void andWaitData(void ** ptr, size_t * ptr_len);

  virtual esp_event_loop_handle_t getEventLoop();

};

/**
 * Templated result expansion to the EWaitGroup
 */
template <typename T>
class TEWaitGroup: public EWaitGroup {
public:

  static_assert( sizeof(T) <= WG_DATA_SIZE, "Struct size bigger than WG_DATA_SIZE" );

  void setResult(T __v0001) {
    this->__v0001 = __v0001;
    this->setResultPtr(&this->__v0001, sizeof(__v0001));
  }

  /**
   * Immediately complete with a given __v0001
   */
  void completeNowWithResult(T __v0001) {
    this->setResult(__v0001);
    this->completeNow();
  }

  /**
   * Call-out to a callback with a given type as __v0001
   */
  void andCallT(std::function<void(T __v0001)> cb) {
    this->andCall([cb](void *event_data, size_t data_len) {
      if ((event_data == NULL) || (sizeof(T) > data_len)) {
        TRACE_LOGE("waitgroup.events", "Obtained __v0001 (%p) is either NULL or has invalid payload size (%zu != %zu)", event_data, data_len, sizeof(T));
        cb(T());
      } else {
        cb(*(T*)event_data);
      }
    });
  }

  /**
   * Synchronously wait and return the __v0001
   */
  T andWaitResult() {
    void * __v0001;
    size_t __v0001_len;
    this->andWaitData(&__v0001, &__v0001_len);

    if ((__v0001 == NULL) || (sizeof(T) > __v0001_len)) {
      TRACE_LOGE("waitgroup.events", "Obtained __v0001 (%p) is either NULL or has invalid payload size (%zu != %zu)", __v0001, __v0001_len, sizeof(T));
      return T();
    } else {
      return *((T*)__v0001);
    }
  }

  /**
   * Overload functions that require size
   */
  void waitForEventFromModuleFn(Module * module, waitEventHandler h) {
    EWaitGroup::waitForEventFromModuleFn(module, h, sizeof(T));
  }
  void waitForEventFromModule(Module * module, int32_t event_id) {
    EWaitGroup::waitForEventFromModule(module, event_id, sizeof(T));
  }
  void waitForEvent(esp_event_base_t event_base, int32_t event_id) {
    EWaitGroup::waitForEvent(event_base, event_id, sizeof(T));
  }
  void waitForGlobalEvent(esp_event_base_t event_base, int32_t event_id) {
    EWaitGroup::waitForGlobalEvent(event_base, event_id, sizeof(T));
  }

private:
  T __v0001;
};

#endif
