#ifndef KUDZUKERNEL_STREAM_WRITER_H
#define KUDZUKERNEL_STREAM_WRITER_H
#include <functional>
#include "esp_system.h"

/**
 * The interface to the function the stream will use to pull data from
 *
 * @param[out]  buf     Pointer to the buffer to send
 * @param[in]   len     The maximum number of bytes allowed to send
 * @param[in]   offset  The offset in the buffer to start reading from
 *
 * @return            Returns a positive number with the bytes desired to send
 *                    or a negative number if an error occurred. Returning 0
 *                    will terminate the stream.
 */
typedef std::function<int(char **, size_t, size_t)> streamChunkFn_t;

/**
 * @brief      A re-usable interface for writing chunked data to a stream
 */
class StreamWriter {
public:

  /**
   * @brief      Starts writing to the stream using the given pull function to
   *             collect the data to send.
   *
   * @param[in]  pullFunction  The pull function
   *
   * @return     The error occurred or E_OK if none.
   */
  virtual esp_err_t writeStreamStart(streamChunkFn_t pullFunction) = 0;

  /**
   * Abort a stream commit
   */
  virtual void writeStreamAbort() = 0;

};

#endif
