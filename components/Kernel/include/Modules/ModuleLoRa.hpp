#ifndef KUDZUKERNEL_MODULELORA_H
#define KUDZUKERNEL_MODULELORA_H
#include <Module.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_pm.h"

/**
 * The exposed event base
 */
extern const esp_event_base_t LORA_EVENT_BASE;

/**
 * Arguments of EVENT_LORA_RX event
 */
struct EventLoraRx_t {
  const char*   data;
  size_t        size;
};

/**
 * Events broadcasted by this module
 */
enum ModuleLoraEvents {
  EVENT_LORA_READY = 0xFF,
  EVENT_LORA_SEND,
  EVENT_LORA_TX_ACK,
  EVENT_LORA_TX_NACK,
  EVENT_LORA_TX_PENDING,
  EVENT_LORA_RX,
};

/**
 * Lora mode enum constant
 */
typedef enum {
  MODULE_LORA_DISABLED = 0, // Disable LoRa capabilities entirely
  MODULE_LORA_TTN_ABP = 1,  // Use The-Things-Network ABP Activation
  MODULE_LORA_TTN_OTAA = 2  // Use The-Things-Network OTAA Activation
} MODULE_LORA_MODE_t;

/**
 * Spreading factor for uNode
 */
typedef enum {
  MODULE_LORA_SF_DEFAULT = 0,
  MODULE_LORA_SF12,
  MODULE_LORA_SF11,
  MODULE_LORA_SF10,
  MODULE_LORA_SF9,
  MODULE_LORA_SF8,
  MODULE_LORA_SF7,
  MODULE_LORA_SF7B
} MODULE_LORA_SPREADFACTOR_t;

/**
 * LoRa module is responsible for activating or de-activating the LoRa on the device
 */
class _ModuleLoRa : public Module {
public:

  /**
   * Return the __v0011 configuration
   */
  virtual const ModuleConfig &getModuleConfig();

  /**
   * Send something over the radio
   *
   * Returns the numbers of bytes sent. 0 indicates an error.
   */
  size_t send(const char *data, size_t len, int port = 1);

  /**
   * Abort message waiting
   */
  esp_err_t abortWaiting();

protected:
  virtual std::vector<ValueDefinition> configOptions();
  virtual void configDidSave();
  virtual void nvsReset(void *nvs);
  virtual void setup();
  DECLARE_EVENT_HANDLER(all_events);
  virtual void activate();
  virtual void deactivate();
private:
  void __v0001();
  void __v0005();
  void __v0003();
  void __v0007();
  static void __v0002(bool isDown, void * ref);
  static void __v000e(bool isDown, void * ref);
  static void __v000f(void * self);
  bool __v0004;
  std::string __v0006, __v000a, __v0008, __v0009, __v000c, __v000d, __v000b;
  TaskHandle_t  __v0012;
  esp_pm_lock_handle_t    __v0010;
};

extern _ModuleLoRa ModuleLoRa;

#endif
