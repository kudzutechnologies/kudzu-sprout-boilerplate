#ifndef KUDZUKERNEL_WITHDIAGNOSTICS_HPP
#define KUDZUKERNEL_WITHDIAGNOSTICS_HPP

/**
 * Module diagnostic info that can be part of the bundle
 */
struct DiagnosticsData {

  /**
   * The size of the data structure
   */
  const size_t  size;

  /**
   * The pointer to the data structure
   */
  const void * data;

  /**
   * The data format
   */
  const uint32_t contentType;

};

/**
 * A decorator that can be placed when a structure is desired to produce
 * diagnostic information.
 */
class WithDiagnostics {
public:

  virtual const DiagnosticsData collectDiagnostics() = 0;

};

#endif
