#ifndef KUDZUKERNEL_WAITGROUPCALLBACK_H
#define KUDZUKERNEL_WAITGROUPCALLBACK_H
#include <functional>

/**
 * WaitGroup Callbacks
 */
typedef std::function<void()> WGConditionCallback;
typedef std::function<void(void *event_data, size_t data_len)> WGTriggerCallback;

#endif
