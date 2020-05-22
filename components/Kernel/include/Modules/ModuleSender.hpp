#ifndef KUDZUKERNEL_ModuleSender_H
#define KUDZUKERNEL_ModuleSender_H
#include <Module.hpp>
#include "Utilities/StaticContainers.hpp"
#include "Utilities/QueueAllocator.hpp"
#include "Utilities/WithFSM.hpp"
#include "Modules/ModuleSARASockets.hpp"

/**
 * The size of the egress queue
 */
#define MODULE_SENDER_EGRESS_QUEUE_SIZE   1024

/**
 * Maximum number of times a message is allowed to be re-tried before
 * considering it aborted.
 */
#define MODULE_SENDER_MAX_RETRIES 3

/**
 * The exposed event base
 */
extern const esp_event_base_t SENDER_EVENT_BASE;

/**
 * Forward declaration of the module singleton
 */
class _ModuleSender;
extern _ModuleSender ModuleSender;

///////////////////////////////////////////
// Declaration of events that can be used
///////////////////////////////////////////

/**
 * An enum that defines the events your modules exchanges
 */
enum ModuleSenderEvents {
  EVENT_SENDER_SEND,
  EVENT_SENDER_SEND_DONE,
  EVENT_SENDER_SEND_NO_CARRIER,
  EVENT_SENDER_DOWNLINK,

  EVENT_SENDER_UPLINK_REQUEST = 0x200,
  EVENT_SENDER_UPLINK_OFFER,
  EVENT_SENDER_UPLINK_DONE,

  EVENT_SENDER_UPLINK_CHUNK,
  EVENT_SENDER_UPLINK_CONTINUE,
  EVENT_SENDER_UPLINK_ERROR,
  EVENT_SENDER_UPLINK_TIMEOUT,
  EVENT_SENDER_UPLINK_RETRY,

  SENDING_LORA_TIMEOUT = 0x100,
  SENDING_LORA_ERROR,

  SENDING_SARA_TIMEOUT,
  SENDING_SARA_ERROR,
  SENDING_SARA_SOCKET_OPEN,
  SENDING_SARA_SOCKET_WRITE,
  SENDING_SARA_SOCKET_READ,
  SENDING_SARA_SOCKET_CLOSE,
  SENDING_SARA_SOCKET_CLOSED,

  __SEND_MESSAGE_TEMP__
};

enum ModulSenderMssageClass {
  /**
   * This message has to be sent ASAP, we don't care about the power impact,
   * but we should choose the channel with the smallest latency.
   *
   * - latency : MIN
   * - power   : <DESC>
   */
  MSGCLASS_CRITICAL,

  /**
   * This message has no particular preference, so we are picking the offer
   * with the smallest latency and power configuration.
   *
   * - latency : <DESC>
   * - power   : <DESC>
   */
  MSGCLASS_DEFAULT,

  /**
   * This message should be sent with the smallest possible power impact. The
   * latency is not important, but the smaller the better, as a second sorting
   * candidate.
   *
   * - latency : <DESC>
   * - power   : MIN
   */
  MSGCLASS_LOWPOWER,
};

typedef int32_t   MessageId_t;
typedef int16_t   MessageSize_t;

struct SenderUplinkRequestEvent {
  uint32_t                size;
  ModulSenderMssageClass  msgClass;
};

struct SenderUplinkDataEvent {
  uint8_t*          data;
  MessageId_t       id;
  MessageSize_t     size;
  MessageSize_t     offset;
  MessageSize_t     remains;
};

enum UplinkCostEnum_t: uint8_t {
  UPLINK_COST_MIN     = 0,
  UPLINK_COST_LOWER   = 1,
  UPLINK_COST_LOW     = 3,
  UPLINK_COST_AVG     = 4,
  UPLINK_COST_HIGH    = 5,
  UPLINK_COST_HIGHER  = 6,
  UPLINK_COST_MAX     = 7,
};

struct SenderUplinkOfferEvent {
  Module*           module;
  uint32_t          eventId;
  uint32_t          eventIdAbort;
  uint32_t          chunkSize;
  uint16_t          timeoutSec;
  uint8_t           costPower : 3;
  uint8_t           costLatency : 3;
  uint8_t           flags : 2;
};

struct SenderDownlinkEvent {
  const char *      data;
  size_t            size;
};

struct MessageMeta_t {
  ModulSenderMssageClass   msgClass;
  MessageId_t              msgId;
};

typedef QueueAllocator<MODULE_SENDER_EGRESS_QUEUE_SIZE, MessageMeta_t>
        ModuleSenderQueueAllocator_t;

///////////////////////////////////////////
// Declaration of the module
///////////////////////////////////////////

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleSender: public Module, public WithFSM {
public:

  _ModuleSender();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

  /**
   * Send some data over the network, choosing the best out of the carriers
   */
  int sendData(const char * data, size_t len, ModulSenderMssageClass msgClass = MSGCLASS_DEFAULT);

private:
  virtual void setup();
  virtual void activate();
  virtual void deactivate();
  DECLARE_EVENT_HANDLER(all_events);
  FSM_DECLARE_HANDLER(Idle);
  FSM_DECLARE_HANDLER(Dequeue);
  FSM_DECLARE_HANDLER(Transmission);
  void __v0006();
  ModuleSenderQueueAllocator_t    __v0009;
  SenderUplinkOfferEvent          __v0004;
  ModuleTimer_t                   __v0003;
  MessageId_t                     __v0007;
  MessageSize_t                   __v0005, __v0008;
  StaticQueue<Module*, 16>        __v0001;
  float                           __v0002;
};


#endif
