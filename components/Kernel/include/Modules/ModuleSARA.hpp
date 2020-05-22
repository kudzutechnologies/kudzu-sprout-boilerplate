#ifndef KUDZUKERNEL_ModuleSARA_H
#define KUDZUKERNEL_ModuleSARA_H
#include <Module.hpp>
#include <queue>
#include "Utilities/WithFSM.hpp"
#include "Modules/ModuleATParser.hpp"

/**
 * Forward declaration of the module singleton
 */
class _ModuleSARA;
extern _ModuleSARA ModuleSARA;

/**
 * The exposed event base
 */
extern const esp_event_base_t SARA_EVENT_BASE;

///////////////////////////////////////////
// Declaration of events that can be used
///////////////////////////////////////////

/**
 * An enum that defines the events your modules exchanges
 */
enum ModuleSARAEvents {
  EVENT_SARA_NETWORK_ACTIVE = 0x0000,
  EVENT_SARA_NETWORK_INACTIVE,
  EVENT_SARA_NETWORK_DETAILS,

  EVENT_SARA_POWERON = 0x0100,
  EVENT_SARA_PROBE,
  EVENT_SARA_PROBE_RESULT,
  EVENT_SARA_OPERATOR_PROBE_RESULT,
  EVENT_SARA_SEQUENCE_NEXT,
  EVENT_SARA_SEQUENCE_RESPONSE,
  EVENT_SARA_NET_PROBE,
  EVENT_SARA_NET_PROBE_RESPONSE,
  EVENT_SARA_IMEI_RESULT,
  EVENT_SARA_CCID_RESULT,
  EVENT_SARA_STOP,
  EVENT_SARA_STOP_RAMPDOWN,
  EVENT_SARA_STOPPED,
  EVENT_SARA_APN_START,
  EVENT_SARA_APN_DISCONNECTED,
  EVENT_SARA_APN_RESPONSE,

  EVENT_SARA_NET_UNREGISTERED,
  EVENT_SARA_NET_REGISTERED,
};

///////////////////////////////////////////
// Declaration of the module
///////////////////////////////////////////

/**
 * WiFi module is responsible for activating or de-activating the WiFi on the device
 */
class _ModuleSARA: public Module, public WithFSM {
public:

  _ModuleSARA();

  /**
   * Return the module configuration
   */
  virtual const ModuleConfig& getModuleConfig();

  /**
   * Checks if the device is connected to the network
   */
  bool isRegistered();

  /**
   * Checks if the APN is up
   */
  bool isAPNActive();

  /**
   * Returns `true` if the network is read for communication
   */
  bool isNetworkActive();

protected:
  virtual void setup();
  DECLARE_EVENT_HANDLER(at_events);
  DECLARE_EVENT_HANDLER(all_events);
  FSM_DECLARE_HANDLER(Init);
  FSM_DECLARE_HANDLER(Configure);
  FSM_DECLARE_HANDLER(Operator);
  FSM_DECLARE_HANDLER(Connect);
  FSM_DECLARE_HANDLER(Stopping);
  virtual std::vector<ValueDefinition> configOptions();
  virtual void configDidSave();
  virtual void nvsReset(void* nvs);
  virtual void fsmWillChange(uint16_t previousId, uint16_t nextId);
  virtual void activate();
  virtual void deactivate();
private:
  void __v0002(bool enabled);
  ModuleTimer_t __v0004;
  std::queue<ATMessageBuffer*> __v0008;
  bool            __v0007;
  char            __v0006[64];
  char            __v0009[24];
  char            __v000a[24];
  int             __v0003;
  int             __v0005;
  bool            __v0001;
};


#endif
