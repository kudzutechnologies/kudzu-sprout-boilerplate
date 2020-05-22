#ifndef KUDZUKERNEL_LOCABLE_H
#define KUDZUKERNEL_LOCABLE_H
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "WaitGroupEvents.hpp"
#include "WaitGroupPool.hpp"

#define LOCKABLE_QUEUE_SIZE   8

/**
 * A class trait that can be used on a module to enable exclusive locking
 */
class Lockable {
public:

  /**
   * Constructor
   */
  Lockable();

  /**
   * Acquire an exclusive lock
   */
  TEWaitGroup<std::function<void()>>* exclusiveLock();

protected:

  /**
   * Called when an exclusive lock is about to be acquired;
   */
  virtual int willLock();

  /**
   * Called when an exclusive lock is just released
   */
  virtual void didUnlock();

private:

  /**
   * Call the next item pending in the lock queue
   */
  void release();

  WaitGroupPool<TEWaitGroup<std::function<void()>>,LOCKABLE_QUEUE_SIZE> lockWgPool;

  SemaphoreHandle_t locksCounter;
  StaticSemaphore_t locksCounterBuffer;

  QueueHandle_t locksQueue;
  StaticQueue_t locksQueueBuffer;
  WGConditionCallback locksQueueStorageBuffer[LOCKABLE_QUEUE_SIZE];

};

#endif
