#ifndef MUDUOZ_NET_EVENTLOOP_H
#define MUDUOZ_NET_EVENTLOOP_H
#include <stdio.h>

#include <boost/noncopyable.hpp>

#include "CurrentThread.h"
using namespace muduoZ;
// one loop per thread
// TODO: auto check when one thread create >1 loop (via __thread)
class EventLoop : boost::noncopyable {
   public:
    EventLoop() : threadId_(::CurrentThread::getTid()) {}
    ~EventLoop(){};

    bool inThreadId_Thread() const { return threadId_ == ::CurrentThread::getTid(); }
    void loop();

   private:
    int threadId_;
};

#endif