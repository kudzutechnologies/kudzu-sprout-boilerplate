#ifndef KUDZUKERNEL_WAITGROUP_HPP
#define KUDZUKERNEL_WAITGROUP_HPP
#include <vector>
#include <queue>
#include <functional>

#include "Module.hpp"
#include "Sherlock.hpp"
#include "WaitGroupCallback.hpp"
#include "esp_log.h"
#include "esp_event.h"

#define WAIT_GROUP_CONDITIONS_SLOTS   4
#define WAIT_GROUP_TRIGGERS_SLOTS     4
#define WAIT_GROUP_DEFAULT_DELAY      100 / portTICK_PERIOD_MS

#define WG_DATA_SIZE                  20

#define WG_OK                         0
#define WG_STOP_PROPAGATION           1

class WGConditions;
typedef std::function<int(void *event_data, size_t data_len)> WGTriggerHandler;
typedef std::function<void(WGConditions&)> WGChainCallback;

/**
 * WaitGroup callbacks logic
 */
class WGConditionSlot {
public:
  uint8_t               data[WG_DATA_SIZE];
  WGConditions *        parent;
  // void complete();
};

class WGConditions {
public:
  WGConditions();
  void reset();
  bool isCompleted();
  bool isStarted();
  void slotComplete(WGConditionSlot * ptr);

  virtual void conditionsCompleted() = 0;

protected:
  friend class WGConditionSlot;
  WGConditionSlot * slotAlloc();
  WGConditionSlot conditions[WAIT_GROUP_CONDITIONS_SLOTS];
  uint8_t n_alloc, n_complete;
};

/**
 * WaitGroup triggers logic
 */
class WGTriggers;
class WGTriggerSlot {
public:
  WGTriggerHandler cb;
};

class WGTriggers {
public:
  WGTriggers();
  void reset();
  bool isCompleted();
  bool isStarted();
  bool callTriggers(void *data, size_t data_len);

  virtual void triggerInstalled() = 0;

protected:
  friend class WGTriggerSlot;
  void slotComplete(WGTriggerSlot * ptr);
  WGTriggerSlot * slotAlloc();
  WGTriggerSlot triggers[WAIT_GROUP_TRIGGERS_SLOTS];
  uint8_t n_alloc, n_complete;
};

/**
 * Base WaitGroup interface class
 */
class WaitGroupInterface: public WGConditions, public WGTriggers {
public:

  /**
   * Default constructor
   */
  WaitGroupInterface();

  /**
   * Set the result to return
   */
  void setResultPtr(void *result, size_t result_len);

  /**
   * Checks if this wait group is free
   */
  bool isFree();

  /**
   * Reset underling state
   */
  void reset();

protected:

  virtual void conditionsCompleted() override;
  virtual void triggerInstalled() override;
  void checkResetCondition();

  void * result;
  size_t result_len;
};

/**
 * Basic wait group that implements only callbacks
 */
class WaitGroup: public WaitGroupInterface {
public:

  /**
   * Return a callback handler that when called will satisfy the condition
   */
  WGConditionCallback waitForCallback();

  /**
   * Immediately complete
   */
  void completeNow();

  /**
   * Will call-out the given callback when the conditions are met
   */
  void andCall(WGTriggerCallback cb);

  /**
   * The chain call can be used as the means to install more triggers as a
   * response to a completion
   */
  void andChain(WGTriggerCallback cb);

  /**
   * Ignore the result but properly GC the operation
   */
  void andIgnore();
};

/**
 * Templated result expansion to the WaitGroup
 */
template <typename T>
class TWaitGroup: public WaitGroup {
public:

  static_assert( sizeof(T) <= WG_DATA_SIZE, "Struct size bigger than WG_DATA_SIZE" );

  void setResult(T result) {
    this->result = result;
    this->setResultPtr(&this->result, sizeof(result));
  }

  /**
   * Immediately complete with a given result
   */
  void completeNowWithResult(T result) {
    this->setResult(result);
    this->completeNow();
  }

  /**
   * Call-out to a callback with a given type as result
   */
  void andCallT(std::function<void(T result)> cb) {
    this->andCall([cb](void *event_data, size_t data_len) {
      if (event_data == NULL || sizeof(T) > data_len) {
        cb(*(T*)event_data);
      } else {
        cb(T());
      }
    });
  }

private:
  T result;

};

#endif
