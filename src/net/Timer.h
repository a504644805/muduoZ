#ifndef MUDUOZ_SRC_NET_TIMER_H
#define MUDUOZ_SRC_NET_TIMER_H

#include <functional>

#include "Timestamp.h"

class Timer {
   public:
    typedef std::function<void()> TimerCallback;
    Timer(const TimerCallback& cb, Timestamp expiration_time) : cb_(cb), expiration_time_(expiration_time) {}
    ~Timer() {}
    void runCb() const { cb_(); }

   private:
    TimerCallback cb_;
    Timestamp expiration_time_;
};

#endif