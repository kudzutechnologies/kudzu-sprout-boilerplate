#ifndef KUDZUKERNEL_ModuleSARASockets_H
#define KUDZUKERNEL_ModuleSARASockets_H
#include <Module.hpp>
#include "Utilities/WaitGroupEvents.hpp"
#include "Utilities/WaitGroupPool.hpp"
#include "Utilities/WithFSM.hpp"
#include "Utilities/AsyncIO.hpp"
#include "Utilities/StaticContainers.hpp"
#include "esp_wifi.h"

/**
 * Maximum number of concurrently open sockets
 */
#define MODULE_SARASOCKETS_MAX_SOCKETS   8

/**
 * Maximum number of sequencing actions
 */
#define MODULE_SARASOCKETS_MAX_SEQUENCER_ACTIONS   16

/**
 * The exposed event base
 */
extern const esp_event_base_t SARA_SOCKET_EVENT_BASE;

enum SARASocketEventStatus {
  SARA_SOCKET_OPEN,
  SARA_SOCKET_CLOSED,
  SARA_SOCKET_ERROR
};

enum SARASocketError {
  SARA_E_NONE = 0,
  SARA_E_ALLOC,           // Unable to open a socket on SARA
  SARA_E_CONNECT,         // Unable to connect to the remote host
  SARA_E_INVALID_STATE,   // Tried to use the socket while closed
  SARA_E_NETWORK_ERROR,   // A network error interrupted the connection
  SARA_E_IO_ERROR,        // The read/write command has failed
  SARA_E_CONFIG_ERROR,    // Unable to change hex configuration mode
  SARA_E_PROTOCOL_ERROR,  // Unable to parse AT protocol format
  SARA_E_NOT_ACTIVE,      // SARA module is not active
};

typedef void* ConnectionHandler_t;

enum SARASocketSlotState {
  SARA_SLOT_FREE,
  SARA_SLOT_PENDING,
  SARA_SLOT_ASSIGNED,
  SARA_SLOT_CONNECTED,
  SARA_SLOT_CLOSING,
  SARA_SLOT_WRITING,
  SARA_SLOT_READING,
};

/**
 * Lightweight copy-able structure that carries
 * useful information for the socket events
 */
struct SARASocketEvent {
  ConnectionHandler_t         handler;
  SARASocketEventStatus       status;
  union {
    struct {
      const char *            data;
      size_t                  length;
    };
    SARASocketError           error;
  };
};

/**
 * A structure used internally by the SARASockets class
 * to keep track of the open sockets.
 */
struct SARASocketSlot {
  SARASocketSlotState         state;
  char                        hostname[64];
  uint16_t                    port;
  uint8_t                     socket_id;
  char *                      data_ptr;
  size_t                      data_len;
};

enum ModuleSARASocketsEvents {
  EVENT_SSOCKET_OPEN,
  EVENT_SSOCKET_OPENED,
  EVENT_SSOCKET_CLOSE,
  EVENT_SSOCKET_CLOSED,
  EVENT_SSOCKET_ERROR,
  EVENT_SSOCKET_DATA_AVAILABLE,

  EVENT_SSOCKET_WRITE,
  EVENT_SSOCKET_WRITE_START,
  EVENT_SSOCKET_DATA_WRITTEN,
  EVENT_SSOCKET_DATA_WRITE,
  EVENT_SSOCKET_READ,
  EVENT_SSOCKET_READ_START,
  EVENT_SSOCKET_DATA_READ,
  EVENT_SSOCKET_SWITCH_BIN_MODE,

  EVENT_SSOCKET_ACK_MODE_HEX_READ,
  EVENT_SSOCKET_ACK_MODE_BIN_READ,
  EVENT_SSOCKET_ACK_MODE_BIN_WRITE,
  EVENT_SSOCKET_ACK_OPEN,
  EVENT_SSOCKET_ACK_CONNECT,
  EVENT_SSOCKET_ACK_WRITTE,
  EVENT_SSOCKET_ACK_WRITE,
  EVENT_SSOCKET_ACK_READ,
  EVENT_SSOCKET_ACK_CLOSE
};

struct SARAPendingAction {
  ModuleSARASocketsEvents     id;
  SARASocketEvent             event;
};

/**
 * WaitGroup specialization for SaraSockets
 */
class SEWaitGroup: public TEWaitGroup<SARASocketEvent> {
public:

  /**
   * Send the following events to the __v0005
   */
  void andSendStatusEventsTo(
    Module* __v0005,
    int32_t __v0004,
    int32_t __v0003,
    int32_t __v0001,
    int32_t __v0002
  );

private:
  Module* __v0005;
  int32_t __v0004;
  int32_t __v0003;
  int32_t __v0001;
  int32_t __v0002;
};

/**
 * Connections manager
 */
class _ModuleSARASockets: public Module, public WithFSM {
public:

  _ModuleSARASockets();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

  /**
   * Opens a connection to the designated host
   */
  SEWaitGroup* socketOpen(const char * hostname, uint16_t port);

  /**
   * Note that the wire and read operation
   */
  SEWaitGroup* socketWrite(ConnectionHandler_t h, const char * data, size_t len);
  SEWaitGroup* socketRead(ConnectionHandler_t h, char * data, size_t len);
  SEWaitGroup* socketClose(ConnectionHandler_t h);

  /**
   * @brief      Returns the data available in the socket
   *
   * @param[in]  h     The socket handler
   *
   * @return     Returns a WaitGroup that is completed with the number of bytes in the rx buffer
   */
  TEWaitGroup<int>* socketDataAvailable(ConnectionHandler_t h);

  /**
   * Shorthand to sequence a socket connection, post and close
   */
  SEWaitGroup* socketSendTo(const char * hostname, uint16_t port, const char * data, size_t len);

protected:
  virtual void setup();
  DECLARE_EVENT_HANDLER(at_events);
  DECLARE_EVENT_HANDLER(sara_events);
  FSM_DECLARE_HANDLER(Disconnected);
  FSM_DECLARE_HANDLER(Connected);
  virtual void activate();
  virtual void deactivate();
private:
  SARASocketSlot * __v0006(SARASocketSlotState state);
  SARASocketSlot * __v0009(int socket_id);
  void __v000a(ModuleSARASocketsEvents type, SARASocketEvent &event);
  void __v0008();
  void __v0007();
  WaitGroupPool<SEWaitGroup, 8>       __v0011;
  WaitGroupPool<TEWaitGroup<int>, 8>  __v000f;
  SARASocketSlot                      __v000c[MODULE_SARASOCKETS_MAX_SOCKETS];
  StaticQueue<SARAPendingAction, MODULE_SARASOCKETS_MAX_SEQUENCER_ACTIONS>   __v000d;
  bool                                                                       __v000b;
  uint8_t  __v0010;
  char __v000e[1024];
};

extern _ModuleSARASockets ModuleSARASockets;

#endif
