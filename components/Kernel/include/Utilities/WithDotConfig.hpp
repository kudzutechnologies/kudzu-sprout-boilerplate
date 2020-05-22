#ifndef KUDZUKERNEL_WITHDOTCONFIG_H
#define KUDZUKERNEL_WITHDOTCONFIG_H
#include <vector>
#include "esp_event.h"
#include "DotConfig/dotconfig/define.hpp"

static const char * DEFAULT_MODULE_PAGE = "Modules";

/**
 * The `WithDotConfig` class provides the base abstraction for providing
 * configurable, visual options to the user.
 */
class WithDotConfig {
public:

  /**
   * Return the title for the configurable section
   */
  virtual const char * configTitle() { return NULL; };

  /**
   * Return the list of configuration options to expose to the user
   * for this module;
   */
  virtual std::vector<ValueDefinition> configOptions() { return {}; };

  /**
   * Return the page name where this module should be placed
   */
  virtual const char * getModulePageName() { return DEFAULT_MODULE_PAGE; };

  /**
   * Called when the user has commited the changes to the module configuration
   */
  virtual void configDidSave() { };

  /**
   * A variable that becomes `true` if the configuration was changed
   */
  bool configChanged;

};

#endif
