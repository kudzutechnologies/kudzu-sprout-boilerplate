#ifndef KUDZUKERNEL_WITHEVENTS_H
#define KUDZUKERNEL_WITHEVENTS_H
class WithEvents;
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"

static const char * __MDBG_TAG = "core.mem";

/**
 * Default value when posting events
 */
#define EVENT_DEFAULT_DELAY               portMAX_DELAY

/**
 * Maximum number of concurrent timers
 */
#define EVENT_MAX_TIMERS                  8

/**
 * Maximum number of data bytes allowed for `eventPostAfter`
 */
#define EVENT_MAX_TIMER_EVENT_DATA        16

/**
 * The event macro defines an in-class handler for IDF events
 * and it's accompanying `*_handler()` function that returns
 * the `esp_event_handler_t` call helper to it.
 *
 * Usage:
 *
 * class ... {
 *   EVENT(some_event) (esp_event_base_t event_base, int32_t event_id, void *event_data) {
 *     ...
 *   }
 * }
 *
 * Or you can break it down in declaration and implementation:
 *
 * class ... {
 *   DECLARE_EVENT(some_event);
 * }
 *
 * DEFINE_EVENT(...::some_event)(esp_event_base_t event_base, int32_t event_id, void *event_data) {
 *
 * }
 */
#define EVENT(NAME) \
  esp_event_handler_t NAME ## _handler () { \
    static auto fn = [](void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) { \
        decltype(this) ref = (decltype(this)) event_handler_arg; \
        ESP_LOGD(__MDBG_TAG, "MemDBG{ event_base=%s, event_id=%d, module=%p, task=%s, stack=%d }", \
            event_base, event_id, ref, pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL)); \
        if (!ref->eventCanReceive()) { \
          ESP_LOGD(ref->eventBase(), "Not accepting event"); \
          return; \
        } \
        ref->NAME(event_base, event_id, event_data); \
      }; \
    return fn; \
  } \
  void NAME

#define EVENT_HANDLER(NAME) \
  EVENT(NAME)

#define DECLARE_EVENT_HANDLER(NAME) \
  EVENT(NAME)(esp_event_base_t event_base, int32_t event_id, void *event_data)

#define DEFINE_EVENT_HANDLER(NAME) \
  void NAME


/**
 * Call-out to ESP to register the event handler previously declared with `EVENT`
 */
#define EVENT_HANDLER_REGISTER(NAME, ID) \
  this->eventHandlerRegister(ID, NAME ## _handler(), this);

#define EVENT_HANDLER_REGISTER_ON(MODULE, NAME, ID) \
  MODULE.eventHandlerRegister(ID, NAME ## _handler(), this);

/**
 * Call-out to ESP to unregister the event handler previously declared with `EVENT`
 */
#define EVENT_HANDLER_UNREGISTER(NAME, ID) \
  this->eventHandlerUnregister(ID, NAME ## _handler());

#define EVENT_HANDLER_UNREGISTER_ON(MODULE, NAME, ID) \
  MODULE.eventHandlerUnregister(ID, NAME ## _handler());

/**
 * The statically allocated timer
 */
struct EventModuleTimer {
  WithEvents      *module;
  TimerHandle_t   timer;
  StaticTimer_t   timer_buffer;
  int32_t         event_id;
  char            event_data[EVENT_MAX_TIMER_EVENT_DATA];
  uint8_t         event_data_size;
};

/**
 * Opaque handler to module timer
 */
typedef void* ModuleTimer_t;

/**
 * The `WithEvents` class provides the base abstraction for implementing
 * classes that can subscribe to the main event loop.
 */
class WithEvents {
public:

  /**
   * The default constructor uses an empty __v0004
   */
  WithEvents();

  /**
   * Return the event base used in this module
   */
  virtual esp_event_base_t eventBase() = 0;

  /**
   * Return `true` if we can receive events (eg. not disabled)
   */
  virtual bool eventCanReceive() = 0;

  /**
   * Dispatch an event on the event base of this device
   */
  void eventPost(int32_t event_id, const void *event_data, size_t event_data_size, const TickType_t ticsk_to_block = EVENT_DEFAULT_DELAY);

  /**
   * Dispatch an event after a designated time has passed
   */
  ModuleTimer_t eventPostAfter(int32_t event_id, const void *event_data, size_t event_data_size, const TickType_t ticks_to_wait);

  /**
   * Register an event handler on the event base of this device
   */
  void eventHandlerRegister(int32_t event_id, esp_event_handler_t handler, void *handler_arg);

  /**
   * Unregister a previously registered event handler
   */
  void eventHandlerUnregister(int32_t event_id, esp_event_handler_t handler);

  /**
   * (Re-)Define the event __v0004
   */
  void eventSetLoop(esp_event_loop_handle_t __v0004);

  /**
   * Stop a previous allocated timer with `eventPostAfter`
   */
  void eventTimerStop(ModuleTimer_t timer);

  /**
   * Stop all running timers
   */
  void eventTimerStopAll();

private:
  static void __v0001(TimerHandle_t xTimer);
  esp_event_loop_handle_t __v0004;
  SemaphoreHandle_t __v0002;
  EventModuleTimer __v0003[EVENT_MAX_TIMERS];
};

#endif
