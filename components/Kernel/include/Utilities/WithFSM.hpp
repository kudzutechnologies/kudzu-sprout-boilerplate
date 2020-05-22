#ifndef KUDZUKERNEL_WITHFSM_H
#define KUDZUKERNEL_WITHFSM_H
#include <functional>
#include "esp_event.h"
#include "WithEvents.hpp"

#define _wfsm_xstr(a) _wfsm_str(a)
#define _wfsm_str(a) #a

const int32_t   EVENT_FSM_STARTED = 0x01ffffff;

typedef std::function<void(esp_event_base_t event_base, int32_t event_id, void *event_data)> FSMStateHandler;

#define FSM_DECLARE_HANDLER(NAME) \
  const uint16_t fsmState ## NAME ## Id = __LINE__; \
  bool fsmIs ## NAME ## Active() { \
    return this->fsmStateId == fsmState ## NAME ## Id; \
  }; \
  FSMStateHandler NAME ## _FSMHandler_get () { \
    auto fn = [this](esp_event_base_t event_base, int32_t event_id, void *event_data) { \
        this->NAME ## _FSMHandler(event_base, event_id, event_data); \
      }; \
    return fn; \
  }; \
  void NAME ## _FSMHandler(esp_event_base_t event_base, int32_t event_id, void *event_data);

#define FSM_DEFINE_HANDLER(NAME) \
  void NAME ## _FSMHandler

/**
 * Subscribe the FSM to the local event stream
 */
#define FSM_HANDLER_REGISTER(EVENT_ID) \
  this->eventHandlerRegister(EVENT_ID, [](void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) { \
    decltype(this) self = (decltype(this)) event_handler_arg; \
    self->_fsmHandleEvent(event_base, event_id, event_data); \
  }, this);

/**
 * Subscribe the FSM to a remote event stream
 */
#define FSM_HANDLER_REGISTER_ON(MODULE, EVENT_ID) \
  MODULE.eventHandlerRegister(EVENT_ID, [](void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) { \
    decltype(this) self = (decltype(this)) event_handler_arg; \
    self->_fsmHandleEvent(event_base, event_id, event_data); \
  }, this);

/**
 * Switches FSM to the given state
 *
 * @param      NAME  The name fof the state where to switch to
 */
#define fsmSetState(NAME) \
  { \
    ESP_LOGD(TAG, "Switching FSM to " _wfsm_xstr(NAME) " (id=%u)", fsmState ## NAME ## Id); \
    _fsmSetStateImpl(NAME ## _FSMHandler_get(), fsmState ## NAME ## Id); \
    uint16_t id = fsmState ## NAME ## Id; \
    this->eventPost(EVENT_FSM_STARTED, &id, sizeof(uint16_t)); \
  }

#define fsmStateActive(NAME) \
  _fsmStateActiveImpl(fsmState ## NAME ## Id)

/**
 * The `WithFSM` class provides extends the base `WithEvents` class and adds
 * an additional, asynchronous state machine helper utilities.
 */
class WithFSM {
protected:
  WithFSM();
  void _fsmHandleEvent(esp_event_base_t event_base, int32_t event_id, void *event_data);
  bool _fsmSetStateImpl(FSMStateHandler state, uint16_t fsmStateId);
  bool _fsmStateActiveImpl(uint16_t fsmStateId);
  virtual void fsmWillChange(uint16_t previousId, uint16_t nextId);
protected:
  FSMStateHandler fsmStateHandler;
  uint16_t fsmStateId;
};


#endif
