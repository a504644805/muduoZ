#ifndef MUDUOZ_SRC_NET_TIMERQUEUE_H
#define MUDUOZ_SRC_NET_TIMERQUEUE_H

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "Timer.h"
#include "Timestamp.h"
class Channel;
class EventLoop;
// TODO:support repeat timer
// TODO:support cancel
class TimerQueue : public boost::noncopyable {
   public:
    // FIXME:Use unique_ptr instead of raw pointer. so we don't need to delete
    // muduo: This requires heterogeneous comparison lookup (N3465) from C++14. so that we can find an T* in a set<unique_ptr<T>>.
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerSet;

    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    void handleRead();  // FIXME: move this function to private.(put it here just want code looks neat)
    // FIXME:Realize thread safe addTimer() via EventLoop::runInloop.(muduo: Must be thread safe. Usually(When???) be called from other threads.)
    void addTimer(const Timer::TimerCallback& cb, Timestamp expiration_time);

    // helper
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

   private:
    int timerfd_;
    std::unique_ptr<Channel> timerfd_channel_;  // TimerQueue is the owner of channel.(Responsibility)

    TimerSet timerSet_;

    EventLoop* loop_;
};

#endif