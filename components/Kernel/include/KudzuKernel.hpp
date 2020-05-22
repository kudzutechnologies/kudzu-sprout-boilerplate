#ifndef KUDZUKERNEL_H
#define KUDZUKERNEL_H
#include <esp_system.h>

#include "Pinout.hpp"
#include "ModuleManager.hpp"

/**
 * In 'BOOT' run-level we are deciding which modules to run
 */
static const int    RUNLEVEL_BOOT = 0;

/**
 * In 'HI-POWER' run-level we are constantly active and fully operational
 */
static const int    RUNLEVEL_EXT_POWER = 1;

/**
 * In 'LOW-POWER' run-level we are doing the bare minimum and sleeping a lot
 */
static const int    RUNLEVEL_BAT_POWER = 2;

/**
 * In 'RECOVERY' run-level we are starting the bare-minimum for a recovery
 */
static const int    RUNLEVEL_RECOVERY = 5;

/**
 * In 'TESTING' run-level we are testing some features
 */
static const int    RUNLEVEL_TESTING = 100;

/**
 * The interface to the KudzuKernel core abstraction
 */
class KudzuKernel {
public:

  /**
   * Version numbers, as populated during build
   */
  static const char *   Version;
  static const char *   VersionGit;
  static const int      VersionNum[3];

  /**
   * The firmware APP ID (can be updated by the user)
   */
  static uint32_t       AppID;

  /**
   * The device unique identifier (obtained by various sources but it's unique
   * per sprout device)
   */
  static uint8_t        DeviceID[6];
  static char           DeviceIDString[13];

  /**
   * Initialize core hardware and start the kernel
   */
  void start(std::vector<Module*> modules);

  /**
   * Enter deep sleep
   */
  void deepSleep(int wakeup_after);

  /**
   * Low-level hardware I/O
   */
  void ledOn();
  void ledOff();
  void ledBlink(const uint16_t on_ms = 500, const uint16_t off_ms = 0, const uint8_t cycles = 1);

  /**
   * Returns `true` if we are booted in a mode at which the RTC memory is
   * preserved (eg. after deep sleep, or after WDT panic)
   */
  bool isRTCMemPerserved();

  /**
   * @brief      Checks if external voltage is supplied
   *
   * @return     True if power good, False otherwise.
   */
  bool isPowerGood();

  /**
   * @brief      Returns the boot reason that caused recovery
   * @details    If there was no recovery since last system boot, this is set
   *             to `ESP_RST_UNKNOWN`
   *
   * @return     Returns the system reset reason
   */
  esp_reset_reason_t recoveryReason();

  /**
   * Returns `true` if the device was restarted after a crash.
   */
  bool isRecoveryBoot();

  /**
   * Send a power profile marker to the debugger interface
   */
  void powerProfileMarker(const char * name);

};

/**
 * System-wide singletons
 */
extern KudzuKernel    Kernel;
extern ModuleManager  Modules;

#endif
