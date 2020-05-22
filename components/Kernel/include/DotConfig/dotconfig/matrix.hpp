#ifndef DOTCONFIG_MATRIX_H
#define DOTCONFIG_MATRIX_H
#include <stdint.h>
#include <string.h>
#include <algorithm>

/**
 * Maximum number of columns a matrix can have
 */
#define DOTCONFIG_MATRIX_MAX_COLS    16

/**
 * Base operations on a matrix
 */
struct DotMatrixInterface {

  /**
   * Iterator access to the matrix elements
   */
  class iterator {
  public:
    explicit iterator(const char* ptr);
    bool operator ==(const iterator &b) const;
    bool operator !=(const iterator &b) const;
    bool operator >(const iterator &b) const;
    bool operator >=(const iterator &b) const;
    bool operator <(const iterator &b) const;
    bool operator <=(const iterator &b) const;
    iterator& operator++();
    const char* operator*();
    bool valid();

  protected:
    const char* ptr;
  };

  virtual iterator      begin() = 0;
  virtual iterator      end() = 0;

  /**
   * Shape-based data iteration
   */
  class rows_iterator: public iterator {
  public:
    explicit rows_iterator(const char* ptr, const uint8_t cols);
    rows_iterator& operator++();
    const char** operator*();
    bool valid();

  protected:
    void    collectRows();
    const   char *nptr;
    const   uint8_t cols;
    const   char* colptrs[DOTCONFIG_MATRIX_MAX_COLS];
  };

  virtual rows_iterator beginRows(const uint8_t cols) = 0;

  /**
   * Data manipulation
   */
  virtual iterator      append(const char * str) = 0;
  virtual iterator      nappend(const char * data, size_t len) = 0;
  virtual void          clear() = 0;

};

/**
 * Implementation of the matrix storage interface with a static stack buffer
 */
template <uint16_t STORAGE_SZ>
struct DotMatrix: public DotMatrixInterface {
private:

  char  buffer[STORAGE_SZ];

  /**
   * Return a pointer to the beginning of the final double-NULL characters
   * that denote the end of the buffer.
   */
  char * endPtr() {
    for (int i=0; i<(STORAGE_SZ-1); ++i) {
      if ((buffer[i] == 0) && (buffer[i+1] == 0)) {
        return &buffer[i];
      }
    }
    return NULL;
  }

public:

  DotMatrix() {
    memset(buffer, 0, STORAGE_SZ);
  }

  DotMatrixInterface::iterator nappend(const char * data, size_t len) {
    char * dst = endPtr();
    size_t used = (size_t)(dst - &buffer[0]) + 1; // (plus tailing null char)
    size_t remains = STORAGE_SZ - used - 1;

    // Check if this actions is going to overrun the buffer
    if ((len + 1) > remains) {
      return DotMatrixInterface::iterator(NULL);
    }

    // Advance past the previous NULL termination, otherwise we are going
    // to append to the previous string
    if (dst != &buffer[0]) dst++;

    // Copy data & mark the new ending
    memcpy(dst, data, len);
    dst[len] = '\0';
    dst[len+1] = '\0';
    return DotMatrixInterface::iterator(dst);
  }

  virtual DotMatrixInterface::iterator append(const char * str) {
    return nappend(str, strlen(str));
  }

  virtual rows_iterator beginRows(const uint8_t cols) {
    return DotMatrixInterface::rows_iterator(buffer, cols);
  }

  virtual DotMatrixInterface::iterator begin() {
    return DotMatrixInterface::iterator(buffer);
  }

  virtual DotMatrixInterface::iterator end() {
    char * ptr = endPtr();
    return DotMatrixInterface::iterator(ptr);
  }

  virtual void clear() {
    buffer[0] = '\0';
    buffer[1] = '\0';
  }

  /**
   * Element accessor (with O(N) complexity)
   */
  const char * get(const int col, const int row) {
    return NULL;
  }

};

#endif
