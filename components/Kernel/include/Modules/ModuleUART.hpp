#ifndef KUDZUKERNEL_MODULEUART_H
#define KUDZUKERNEL_MODULEUART_H
#include <Module.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "Interfaces/StreamWriter.hpp"

class _ModuleUART;

/**
 * Configuration parameters
 */
#define UART_RX_BUFFER            2048
#define UART_TX_BUFFER            0
#define UART_ISR_QUEUE_SIZE       24
#define UART_PATTERN_QUEUE_SIZE   24

enum ModuleUARTEvents {
  EVENT_UART_RX_PATTERN,
  EVENT_UART_RX_DATA,
  EVENT_UART_TX_DATA
};

struct ModuleUARTRxEvent {
public:
  ModuleUARTRxEvent(_ModuleUART *__v0002);
  size_t consume(char * buffer, size_t len);
  size_t available;
  bool usePat;
private:
  size_t __v0001(size_t len);
  _ModuleUART *__v0002;
};

/**
 * The UART module is the base class implemented by UART1 or UART2 modules
 * and provides the packet-level abstraction for exchanging data with the ports
 */
class _ModuleUART: public Module {
public:

  /**
   * Constructor
   */
  _ModuleUART();

  /**
   * Filled-in by the implementation;
   */
  uart_port_t     uart_port;
  uart_config_t   uart_config;
  int             tx_pin, rx_pin;
  char            pattern;
  bool            uninstall_existing;

  /**
   * Enable or disable hardware pattern detection
   */
  void patternDetectSet(const char c);
  void patternDetectClear();

  /**
   * Direct call to send data
   */
  size_t write(const char * ptr, size_t data);

  /**
   * @brief      Starts writing to the stream using the given pull function to
   *             collect the data to send.
   *
   * @param[in]  __v0005  The pull function
   */
  virtual esp_err_t writeStreamStart(streamChunkFn_t __v0005);

  /**
   * Abort a stream commit
   */
  virtual void writeStreamAbort();

protected:
  static void __v0004(void *self);
  virtual void activate();
  virtual void deactivate();
private:
  QueueHandle_t   __v0007;
  bool            __v0003;
  TaskHandle_t    __v0008;
  streamChunkFn_t __v0005;
  size_t          __v0006;
};

#endif
