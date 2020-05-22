#ifndef KUDZUKERNEL_POSTMORTEM_H
#define KUDZUKERNEL_POSTMORTEM_H

/**
 * Sherlock is the debugging core of KudzuKernel, capable of collecting post-mortems
 * and forwarding them for debugging.
 */

#include "Errors.hpp"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PANIC(EXPR) \
  { sherlock_trace(0x86); \
   sherlock_panic( __FILE__, __LINE__, (uint32_t)(EXPR) ); }

#define ESP_ERROR_PANIC(EXPR) \
  { uint32_t __errno = (EXPR); \
    if (__errno != ESP_OK) { \
      sherlock_trace(0x86); \
      sherlock_panic( __FILE__, __LINE__, E_ESP_ERROR | __errno); \
    } \
  }

#define OOPS(EXPR) \
  { sherlock_trace(0x85); \
    sherlock_oops( __FILE__, __LINE__, (uint32_t)(EXPR) ); }

#define ESP_ERROR_OOPS(EXPR) \
  { uint32_t __errno = (EXPR); \
    if (__errno != ESP_OK) { \
      sherlock_trace(0x85); \
      sherlock_oops( __FILE__, __LINE__, E_ESP_ERROR | __errno); \
    } \
  }

#define TRACE(ACTION) \
  sherlock_trace( ACTION & 0x7F )

#define TRACE_LOGE( tag, format, ... ) \
  { sherlock_trace(0x84); \
    ESP_LOG_LEVEL_LOCAL(ESP_LOG_ERROR,   tag, format, ##__VA_ARGS__); }

#define TRACE_LOGW( tag, format, ... ) \
  { sherlock_trace(0x83); \
    ESP_LOG_LEVEL_LOCAL(ESP_LOG_WARN,   tag, format, ##__VA_ARGS__); }

#define TRACE_LOGI( tag, format, ... ) \
  { sherlock_trace(0x82); \
    ESP_LOG_LEVEL_LOCAL(ESP_LOG_INFO,   tag, format, ##__VA_ARGS__); }

#define TRACE_LOGD( tag, format, ... ) \
  { sherlock_trace(0x81); \
    ESP_LOG_LEVEL_LOCAL(ESP_LOG_DEBUG,   tag, "CPU %d: " format, xPortGetCoreID(), ##__VA_ARGS__); }

#define TRACE_LOGV( tag, format, ... ) \
  { sherlock_trace(0x80); \
    ESP_LOG_LEVEL_LOCAL(ESP_LOG_VERBOSE,   tag, format, ##__VA_ARGS__); }

#define S_LOGI(TAG, FORMAT, ...) \
  sherlock_trace(); \
  ESP_LOGI(TAG, FORMAT, __VA_ARGS__); \


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Called as early as possible and collects the boot information for the device
 */
void sherlock_setup();

/**
 * Sends a post-mortem using any currently activated device (making sure
 * never to allocate any memory) and then halts the CPU.
 */
void sherlock_panic(const char * file, uint32_t line, const uint32_t error);
void sherlock_oops(const char * file, uint32_t line, const uint32_t error);

/**
 * Marks a particular code segment as "visited", that can be used for debugging
 * the program execution at a later state.
 */
void sherlock_trace(const uint8_t action);

#ifdef __cplusplus
}
#endif

#endif
