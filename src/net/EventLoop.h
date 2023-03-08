#ifndef MUDUOZ_NET_EVENTLOOP_H
#define MUDUOZ_NET_EVENTLOOP_H
#include <stdio.h>

#include <memory>
#include <mutex>

#include "CurrentThread.h"
#include "Poller.h"
#include "Timer.h"
#include "noncopyable.h"

class Channel;
class TimerQueue;
using namespace muduoZ;
// one loop per thread
// TODO: auto check when one thread create >1 loop (via __thread)
class EventLoop : muduoZ::noncopyable {
   public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();

    bool inThreadId_Thread() const { return threadId_ == ::CurrentThread::getTid(); }
    void loop();

    // helper
    void updateChannel(Channel* channel) { poller_.updateChannel(channel); }
    void removeChannel(Channel* channel) { poller_.removeChannel(channel); }

    void runAt(Timestamp time, Timer::TimerCallback cb);
    void runAfter(double delay, Timer::TimerCallback cb);

    // copy from muduo
    void runInLoop(Functor cb);    // Different thread all call same loop's runInLoop.
    void queueInLoop(Functor cb);  // Hence must be thread-safe
    void wakeup();
    void handleRead();  // handle readable event of wakeupFd (Do noting, just want go to doPendingFunctors)
    void doPendingFunctors();

    void quit() { quit_ = true; }

   private:
    int threadId_;
    Poller poller_;
    static const int kPollTimeMs = 10000;

    std::unique_ptr<TimerQueue> timerQueue_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;  // GUARDED_BY(mutex_)
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    bool callingPendingFunctors_;

    bool quit_;

    // helper
    std::vector<Channel*> activeChannels_;
    void printActiveChannels() const;
};

#endif