#ifndef KUDZUKERNEL_MODULE_H
#define KUDZUKERNEL_MODULE_H
class Module;

#include "Utilities/WithEvents.hpp"
#include "Utilities/WithNVS.hpp"
#include "Utilities/WithDotConfig.hpp"
#include "Utilities/WaitGroupCallback.hpp"
#include "Utilities/WithDiagnostics.hpp"

/**
 * Bit flags that can be used to compose a value for ModuleConfig::activate
 */
#define DEFAULT_INACTIVE      0
#define DEFAULT_ACTIVE        1
#define USER_ACTIVATE         2

/**
 * The default module category
 */
extern const char* MODULE_CATEGORY_DEFAULT;
extern const char* MODULE_CATEGORY_NETWORK;
extern const char* MODULE_CATEGORY_DRIVER;
extern const char* MODULE_CATEGORY_SYSTEM;
extern const char* MODULE_CATEGORY_SENSOR;

/**
 * Module configuration
 */
struct ModuleConfig {

  /**
   * The name of the module. This name will be present on the log and
   * configuration UI and must be less than 15-characters long!
   */
  const char * name;

  /**
   * The title of this module (for the UI)
   */
  const char * title;

  /**
   * The module category
   */
  const char * category;

  /**
   * The size of the non-volatile storage space to allocate for the module
   * If this value is set to '0' no NV storage will be allocated.
   */
  size_t nv_size;

  /**
   * The version of the non-volatile storage.
   * If this version changes, the NV storage is reset
   */
  uint32_t nv_version;

  /**
   * The run-levels where this module is activated implicitly by
   * the module manager.
   *
   * Note that the module can still be enabled by the user at any point
   * in time.
   */
  std::vector<uint8_t> runlevels;

  /**
   * Loadable configuration
   *
   * 0 - The module is not user-loadable and will not be activated at start-up
   * 1 - The module is not user-loadable, but will be activated at start
   * 2 - The module is user-loadable and not activated by default
   * 3 - The module is user-loadable and activated by default
   */
  uint8_t activate;

  /**
   * Specifies a list of modules this module depends on, and will be activated
   * before activating the module.
   */
  std::vector<Module*> depends;

};

/**
 * A Module is a dynamically loadable run-time component that is loaded,
 * activated or de-activated on demand.
 */
class Module: public WithEvents, public WithNVS, public WithDotConfig, public WithDiagnostics {
public:

  /**
   * Initialize the module and bind it to the given event loop system
   */
  Module();

  /**
   * Return the number of current dependencies of this module
   */
  virtual uint8_t getUses();

  /**
   * Checks if the module is active
   */
  virtual bool isActive();

  /**
   * Returns `true` if the module is not doing any work and can therefore
   * be turned off.
   */
  virtual bool isIdle();

  /**
   * Override the `eventBase` function from the `WithEvents` class, in order
   * to proxy the module name as the event base.
   */
  virtual esp_event_base_t eventBase();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig() = 0;

protected:

  /**
   * Return the page name where this module should be placed
   */
  virtual const char * getModulePageName();

  /**
   * Collect and return module diagnostic meta-data that will be part
   * of the diagnostics bundle.
   */
  virtual const DiagnosticsData collectDiagnostics();

  /**
   * Return `true` if we can receive events (eg. not disabled)
   */
  virtual bool eventCanReceive();

  /**
   * Return the size of the non-volatile storage required
   */
  virtual size_t nvsSize();

  /**
   * Return the namespace of the local NVS storage
   */
  virtual const char * nvsNS();

  /**
   * Called when the user has commited the changes to the module configuration
   */
  virtual void configDidSave();

  /**
   * Return the version of the NVS storage
   */
  virtual uint32_t nvsVersion();

  /**
   * Return the title for the configurable section
   */
  virtual const char * configTitle();

  /**
   * Called once the module is __v0007ed and can be used to configure the
   * peripherals this module controls. Use this instead of the constructor.
   *
   * The default implementation of this function is to do nothing.
   */
  virtual void setup();

  /**
   * Called when the system is going to sleep and allows the module to prepare
   * it's peripherals for this condition and/or to register an interrupt source.
   *
   * The default implementation of this function is to do nothing.
   */
  virtual void shutdown();

  /**
   * Called when the module is requested to be activated. The module
   * is allowed to perform it's activation routine asynchronously and when
   * ready it should call the `ackActivate` protected method.
   *
   * The default implementation of this function is to do nothing.
   */
  virtual void activate();

  /**
   * Called when the module is requested to be deactivated. The module
   * is allowed to perform it's deactivation routine asynchronously and when
   * ready it should call the `ackDeactivate` protected method.
   *
   * The default implementation of this function is to do nothing.
   */
  virtual void deactivate();

  /**
   * Handler for system events, dispatched via the old-school event loop
   */
  virtual esp_err_t handleSystemEvent(system_event_t *event);

  /**
   * Acknowledge the activation of the device
   */
  void ackActivate();

  /**
   * Acknowledge the deactivation of the device
   */
  void ackDeactivate();

  /**
   * Set the __v0008 state of the module
   */
  void moduleSetIdle(bool __v0008 = true);

  /**
   * Set the busy state of the module (inverse of setIdle)
   */
  void moduleSetBusy(bool busy = true);

private:
  uint8_t __v0004(int8_t v);
  WGConditionCallback __v0002, __v0001;
  bool __v0007;
  bool __v0005;
  bool __v0008;
  uint8_t __v0006;
  uint8_t __v0003;
  friend class ModuleManager;
  friend class DiagnosticsCollector;
};

#endif
