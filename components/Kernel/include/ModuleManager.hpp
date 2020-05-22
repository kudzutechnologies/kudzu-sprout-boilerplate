#ifndef KUDZUKERNEL_MODULEMANAGER_H
#define KUDZUKERNEL_MODULEMANAGER_H
#include <vector>
#include <functional>
#include "esp_event.h"
#include "esp_task.h"
#include "nvs.h"

// Source module only after forward-declaring manager
class ModuleManager;
#include "Module.hpp"
#include "Utilities/WithEvents.hpp"
#include "Utilities/WaitGroupEvents.hpp"
#include "Utilities/WaitGroupPool.hpp"
#include "Utilities/StaticContainers.hpp"

/**
 * The total number of modules the module manager can process
 * adjust this to match your module requirements.
 */
#define MODULE_MANAGER_MAX_MODULES      48

/**
 * The total number of messages that can exist in the system loop queue
 */
#define MODULE_MANAGER_QUEUE_SIZE       MODULE_MANAGER_MAX_MODULES * 8

/**
 * How many concurrent asynchronous operations we are expecting to have.
 * For instance, concurrent `activate`, `restart` or `deactivate` ops.
 */
#define MODULE_MANAGER_MAX_ASYNC_OPS    8

/**
 * The stack size for the module event loop (in words, so x2 in bytes)
 */
#define MODULE_MANAGER_EVENTLOOP_STACK  8192

/**
 * The system event loop task priority
 */
#define MODULE_MANAGER_EVENTLOOP_PRIORITY 2

/**
 * Maximum number of concurrent ISR events that can be queued
 */
#define MODULE_MANAGER_MAX_ISR_EVENTS     8

/**
 * Maximum number of data bytes allowed for `eventPostFromISRTo`
 */
#define MODULE_MANAGER_MAX_ISR_EVENTDATA  8

/**
 * Maximum number of shutdown handlers that can be registered in the system
 */
#define MODULE_MANAGER_MAX_SHUTDOWN_HANDLERS  8

/**
 * The module manager event base
 */
extern const esp_event_base_t MM_EVENT_BASE;

/**
 * In 'SHUTDOWN' run-level has everything disabled, and when
 * reached the system automatically enters deep sleep.
 */
static const int    RUNLEVEL_SHUTDOWN = 0xFF;

/**
 * In 'RESTART' run-level has everything disabled, and when
 * reached the system automatically soft-restarts.
 */
static const int    RUNLEVEL_RESTART = 0xFE;

/**
 * The signature of the function to register during shutdown
 */
typedef std::function<void()>   ShutdwnHandler_t;

/**
 * The structure in the ISR queue
 */
 struct ModuleISREvent {
  WithEvents      *module;
  int32_t         event_id;
  char            event_data[MODULE_MANAGER_MAX_ISR_EVENTDATA];
  uint8_t         event_data_size;
 };

/**
 * Events broadcasted by the module manager
 */
enum ModuleManagerEvents {
  /**
   * Broadcasted when the system has reached the designated runlevel
   */
  EVENT_MM_SYSTEM_RUNLEVEL_CHANGING,
  EVENT_MM_SYSTEM_RUNLEVEL_REACHED,
  EVENT_MM_SYSTEM_IDLE,
  EVENT_MM_SYSTEM_BUSY,
  EVENT_MM_MODULE_ACTIVATED,
  EVENT_MM_MODULE_DEACTIVATED,
  EVENT_MM_MODULE_IDLE_CHANGED,

  EVENT_MM_SEQUENCE_ACTIVATE_CONTINUE = 0x100,
  EVENT_MM_SEQUENCE_ACTIVATE_COMPLETED,
  EVENT_MM_SEQUENCE_DEACTIVATE_CONTINUE,
  EVENT_MM_SEQUENCE_DEACTIVATE_COMPLETED,
};

enum mm_timeout_mode_t {
  MM_TIMEOUT_NONE,
  MM_TIMEOUT_ACTIVATE,
  MM_TIMEOUT_DEACTIVATE
};

/**
 * Module Manager takes care of initializing and controlling the life-cycle
 * of system modules.
 */
class ModuleManager: public WithEvents {
public:

  /**
   * Initialize the module manager with an initial set of __v0018
   */
  ModuleManager(std::vector<Module*> __v0018);

  /**
   * Dynamically install an additional module
   */
  void installModule(Module* module);

  /**
   * Initialize module manager and all the currently loaded
   * __v0018. After calling this method any further module addition
   * will be automatically configured.
   *
   * IMPORTANT: NV-Store is assumed to be already activated!
   */
  void setup();

  /**
   * Set the current system run-level to the designated level. This will
   * start whichever __v0018 are supposed to run on this level, and stop __v0018
   * that are not supposed to run on this level.
   *
   * This is very similar to the linux boot process. Typically __v0016 0 in our
   * case is the 'configuration' level, while level 1 is the 'run' level.
   * Optionally, level '2' could be used for 'low-power' set-up.
   *
   * This function returns the `WaitGroup` object that can be used to check
   */
  TEWaitGroup<int>* setRunlevel(uint8_t level);

  /**
   * Returns `true` if we are in the process of chaning levels
   */
  bool isChangingLevels();

  /**
   * Returns `true` if all __v0018 are IDLE
   */
  bool isIdle();

  /**
   * Get the current system __v0016
   */
  uint8_t getRunlevel();

  /**
   * Restart the designated module by de-activating and re-activating it
   */
  TEWaitGroup<int>* restart(Module *module);

  /**
   * Activate one or more __v0018
   *
   * This function returns the `WaitGroup` object that is bound to this
   * operation.
   */
  TEWaitGroup<int>* activate(Module *module);
  TEWaitGroup<int>* activateAll(std::vector<Module*> __v0018);

  /**
   * Deactivate one or more __v0018
   *
   * This function returns the `WaitGroup` object that is bound to this
   * operation.
   */
  TEWaitGroup<int>* deactivate(Module *module);
  TEWaitGroup<int>* deactivateAll(std::vector<Module*> __v0018);

  /**
   * Deactivate all active __v0018
   */
  TEWaitGroup<int>* deactivateAllActive();

  /**
   * Return the UI pages for the loaded __v0018
   */
  std::vector<PageDefinition> getModuleConfigPages();

  /**
   * Called by the UI to apply the UI changes
   */
  void configDidSave();

  /**
   * Return the handler to the system event __v001a
   */
  esp_event_loop_handle_t getSystemEventLoop();

  /**
   * Post an event to a module from an ISR
   */
  void eventPostFromISRTo(Module * module, int32_t event_id, const void *event_data, size_t event_data_size);

  /**
   * Register a shutdown handler
   */
  void registerShutdownHandler(ShutdwnHandler_t handler);

  /**
   * Reset the state of all the __v0018 declared as degraded in NVS
   */
  void resetDegradedModuleState();

private:
  friend class DiagnosticsCollector;
  virtual esp_event_base_t eventBase();
  virtual bool eventCanReceive();
  static void __v0011(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
  static esp_err_t __v0005(void *ctx, system_event_t *event);
  static void __v000c(void *ctx);
  static void __v000b(Module * m, nvs_handle handle);
  static void __v0009(Module * m, nvs_handle handle);
  static bool __v000d(Module *module);
  void __v0013(Module* module, nvs_handle handle);
  void __v0006(Module* module, nvs_handle handle = 0);
  void __v0008(Module* module, int refs);
  void __v0004(Module* module, int refs);
  void __v0002(uint8_t newLevel);
  void __v0001(uint8_t newLevel);
  void __v0003();
  esp_event_loop_handle_t __v001a;
  void __v000e(int32_t event_id, const void *event_data, size_t event_data_size);
  WaitGroupPool<TEWaitGroup<int>,MODULE_MANAGER_MAX_ASYNC_OPS> __v0019;
  Module*           __v0018[MODULE_MANAGER_MAX_MODULES];
  QueueHandle_t     __v0017;
  bool              __v0012;
  uint8_t           __v000f;
  uint8_t           __v0016;
  bool              __v000a;
  bool              __v0010;
  int64_t           __v0014;
  mm_timeout_mode_t __v0015;
  StaticQueue<ShutdwnHandler_t, MODULE_MANAGER_MAX_SHUTDOWN_HANDLERS> __v0007;
};

/**
 * Singleton of the module manager
 */
extern ModuleManager Modules;

#endif
