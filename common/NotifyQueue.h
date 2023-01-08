#ifndef _COMMON_NOTIFYQUEUE_H_
#define _COMMON_NOTIFYQUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>

#include "StaticQueue.h"

template <typename T, size_t N = 1>
class NotifyQueue
{
private:

public:
    NotifyQueue();
    ~NotifyQueue();

};

#endif