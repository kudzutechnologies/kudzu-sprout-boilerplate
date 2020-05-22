#ifndef KUDZUKERNEL_MODULEATPARSER_H
#define KUDZUKERNEL_MODULEATPARSER_H
class _ModuleATParser;
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include <Module.hpp>
#include "Modules/ModuleUART.hpp"
#include "Utilities/StaticContainers.hpp"
#include "Utilities/WaitGroupEvents.hpp"
#include "Utilities/WaitGroupPool.hpp"
#include "Utilities/WithFSM.hpp"
#include "Utilities/ATMessageBuffer.hpp"
#include "esp_wifi.h"

/**
 * Default value when posting events
 */
#define MODULE_ATPARSER_MESSAGE_TIMEOUT     15000 / portTICK_PERIOD_MS

/**
 * Maximum number of concurrent `sendMessage` commands that can be pending.
 *
 * This must be at least 2 to accommodate continuous chaining of `sendMessage`
 * commands.
 */
#define MODULE_ATPARSER_MAX_MESSAGE_QUEUE  16

extern const esp_event_base_t ATPARSER_EVENT_BASE;

enum ModuleATParserEvents {
  EVENT_AT_RESPONSE,
  EVENT_AT_UNSOLICITED,
  EVENT_AT_DATA_REQUESTED,

  EVENT_AT_SEND = 0x100,
  EVENT_AT_TIMEOUT,
  EVENT_AT_DATA_SEND,
  EVENT_AT_DATA_RECEIVED,
};

enum ATResponseEventStatus {
  AT_RESPONSE_STATUS_OK = 0,
  AT_RESPONSE_STATUS_ERROR,
  AT_RESPONSE_STATUS_ABORTED,
  AT_RESPONSE_STATUS_TIMEOUT,
  AT_RESPONSE_STATUS_UNSOLICITED,

  // Plain commands append the value right away
  AT_REQUEST_PLAIN = 0,
  // Extended AT commands assign the value with '='
  AT_REQUEST_TYPE_EXTENDED,
  // Collect the entire response and not just the response
  // to the command.
  AT_RESPONSE_ALL = 0x10,
};

enum ModuleATParserATState {
  AT_IDLE = 0,
  AT_WAITING_RESPONSE,
  AT_BUFFERING_RESPONSE,
};

struct ATDataPointer {
  char *    data;
  size_t    data_len;
};

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleATParser: public Module, public WithFSM {
public:

  _ModuleATParser();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

  /**
   * Allocate a new message buffer from pool
   */
  ATMessageBuffer * newMessage();

  /**
   * Allocate a new message buffer using a given buffer
   */
  ATMessageBuffer * newMessageWithBuffer(char * buf, size_t len);

  /**
   * Send a message and wait for response
   */
  TEWaitGroup<ATMessageBuffer*>*  sendMessage(ATMessageBuffer* msg);

  /**
   * Suspend or resume operation
   */
  void setSuspend(bool __v0005);

protected:
  virtual void setup();
  DECLARE_EVENT_HANDLER(local_events);
  FSM_DECLARE_HANDLER(Idle);
  FSM_DECLARE_HANDLER(Send);
  FSM_DECLARE_HANDLER(ReceiveResponse);
  FSM_DECLARE_HANDLER(ReceiveData);
  virtual void activate();
  virtual void deactivate();
  void __v0002();
  void __v0006(ModuleUARTRxEvent *rx, size_t consumed);
private:
  ATMessageBuffer               *__v0007;
  ATMessageBuffer               __v0004[MODULE_ATPARSER_MAX_MESSAGE_QUEUE];
  SemaphoreHandle_t             __v0001;
  ModuleTimer_t                 __v0003;
  bool                          __v0005;
  StaticQueue<ATMessageBuffer*, MODULE_ATPARSER_MAX_MESSAGE_QUEUE> __v0009;
  WaitGroupPool<TEWaitGroup<ATMessageBuffer*>, MODULE_ATPARSER_MAX_MESSAGE_QUEUE> __v0008;
};

extern _ModuleATParser ModuleATParser;

#endif
