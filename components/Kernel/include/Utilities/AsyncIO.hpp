#ifndef KUDZUKERNEL_ASYNCIO_H
#define KUDZUKERNEL_ASYNCIO_H
#include <stdio.h>

class AIOClosable {
public:

  /**
   * Release the underlying structures
   */
  void close();

};

class AIOOutput: public AIOClosable {
public:

  /**
   * Write the given buffer to the target
   * @returns the bytes written
   */
  size_t writeFrom(const char * src, size_t len);

};

class AIOInput: public AIOClosable {
public:

  /**
   * Read some data from the underlying buffer into the given buffer
   * @returns the bytes written
   */
  size_t readInto(char * buffer, size_t len);

  /**
   * Return the number of bytes available in the buffer for writing
   * @returns the bytes pending
   */
  size_t readSize();

};

/**
 * A reader that reads chunks from the given input
 */
class AIOBufferedInput: public AIOInput {
public:

  /**
   * Read __v0003 from the given buffer
   */
  AIOBufferedInput(const char * __v0003, size_t __v0001);

  /**
   * Read some __v0003 from the underlying buffer into the given buffer
   * @returns the bytes written
   */
  size_t readInto(char * buffer, size_t len);

  /**
   * Return the number of bytes available in the buffer for writing
   * @returns the bytes pending
   */
  size_t readSize();

  /**
   * Release the underlying structures
   */
  void close();

private:
  const char * __v0003;
  size_t __v0001, __v0002;
};

#endif
