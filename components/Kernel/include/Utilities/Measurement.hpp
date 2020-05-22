#ifndef KUDZUKERNEL_MEASUREMENT_HPP
#define KUDZUKERNEL_MEASUREMENT_HPP
#include <vector>
#include <array>
#include <stdint.h>
#include <functional>
#include <sdkconfig.h>
#include "Utilities/Variant.hpp"
#include "Utilities/StaticContainers.hpp"

/**
 * An identifier represented either by a constant string, a number or both
 */
struct PathIdentifier {
  const char *  name;
  uint32_t      id;
};

/**
 * A measurement value is a key/value tuple
 */
struct MeasurementValue {
  const char *  name;
  Variant       value;
};

/**
 * Release helper
 */
typedef std::function<void(void)> measurement_release_fn;

/**
 * A measurement value as collected by a sensor group
 */
struct Measurement {
  StaticVector<const PathIdentifier, 4 >  path;
  StaticVector<const char*, 8 >           tags;
  StaticVector<MeasurementValue, 8 >    values;
  measurement_release_fn                                                        release;

  /**
   * Try to merge with the given measurement and if it cannot be merged,
   * returns false.
   */
  bool mergeWith(const Measurement * m, bool matchTags = true);

  /**
  * Return the measurement name as string
   */
  const char * name() const;

  /**
   * Lookup a measurement value by name
   */
  Variant getValue(const char * name);

  /**
   * Checks if value exists
   */
  bool hasValue(const char * name);
};

/**
 * Prints the measurement representation on the terminal
 */
void TRACE_MEASUREMENT(const char * TAG, const Measurement * m);

#endif
