#ifndef KUDZUKERNEL_ATPARSER_MESSAGEBUFFER
#define KUDZUKERNEL_ATPARSER_MESSAGEBUFFER
#include <stdint.h>

/**
 * The size of the small buffer in the message structure
 */
#define ATMESSAGE_SMALL_BUFFER_SIZE   64

/**
 * @brief Possible flags for the message type
 */
enum ATMessageType_t: uint8_t {
  /**
   * Performs no transformation between command and value
   */
  AT_PLAIN = 0x00,

  /**
   * Appends "?" to the AT command
   */
  AT_QUERY = 0x01,

  /**
   * Appends "=<value>" to the given command
   */
  AT_WITH_VALUE = 0x02,

  /**
   * Denote that the request (or response) includes binary data
   */
  AT_BINARY = 0x80,
};

/**
 * @brief Possible flags for the message response status
 */
enum ATMessageStatus_t: uint8_t {
  /**
   * The command was acknowledged
   */
  AT_RESPONSE_OK,

  /**
   * The command failed
   */
  AT_RESPONSE_ERROR,

  /**
   * The command was aborted
   */
  AT_RESPONSE_ABORT,

  /**
   * The command timed out
   */
  AT_RESPONSE_TIMEOUT,

  /**
   * Unsolicited message
   */
  AT_RESPONSE_UNSOLICITED,

  /**
   * The response is binary
   */
  AT_RESPONSE_BINARY = 0x80
};

/**
 * Statically allocated buffer for AT messages, that allows
 * dynamic split between a command and a value.
 */
class __v0001 {
public:
  union {
    ATMessageStatus_t status;
    ATMessageType_t   type;
  };

  const char * getCommand();
  const char * getValue();
  size_t getCommandSize();
  size_t getValueSize();

  void define(const char * cmd, const char * value = "");
  void definef(const char * cmd, const char * format, ... );
  void ndefine(const char * cmd, size_t __v0008, const char * value, size_t __v0007);
  void clearValue();
  void setValue(const char * value);
  void setNValue(const char * value, size_t __v0007);
  void appendValue(const char * value);
  void appendNValue(const char * value, size_t __v0007);
  void release();

private:
  friend class _ModuleATParser;
  __v0001();
  __v0001(char * __v000a, size_t len);
  char    __v0009[ATMESSAGE_SMALL_BUFFER_SIZE];
  char    *__v000a;
  void      *__v0006;
  size_t    __v0005;
  uint16_t  __v0008;
  uint16_t  __v0007;
  uint8_t   __v000c;
  char * __v0003();
  size_t __v0002();
  void __v0004(const char * characters);
  void __v000b();
};

#endif
