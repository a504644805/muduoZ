#ifndef MUDUOZ_NET_EVENTLOOP_H
#define MUDUOZ_NET_EVENTLOOP_H
#include <stdio.h>

#include <boost/noncopyable.hpp>

#include "CurrentThread.h"
#include "Poller.h"
class Channel;
using namespace muduoZ;
// one loop per thread
// TODO: auto check when one thread create >1 loop (via __thread)
class EventLoop : boost::noncopyable {
   public:
    EventLoop() : threadId_(::CurrentThread::getTid()) {}
    ~EventLoop(){};

    bool inThreadId_Thread() const { return threadId_ == ::CurrentThread::getTid(); }
    void loop();

    // helper
    void updateChannel(Channel* channel) { poller_.updateChannel(channel); }
    void removeChannel(Channel* channel) { poller_.removeChannel(channel); }

   private:
    int threadId_;

    // helper
    Poller poller_;
    std::vector<Channel*> activeChannels_;
    void printActiveChannels() const;
};

#endif