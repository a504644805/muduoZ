#include "Thread.h"

#include "CurrentThread.h"

void Thread::start() {
    thread_.reset(new std::thread([&]() {
        // child thread is running
        started_ = true;
        pthreadId_ = thread_->get_id();
        tid_ = muduoZ::CurrentThread::getTid();  // now we get the tid
        latch_.countDown();
        func_();
    }));

    latch_.wait();  // parent thread waits till child finish preparing tid_
}

void Thread::join() {
    assert(started_ && !joined_);  // code logic err
    thread_->join();
    joined_ = true;
}

Thread::~Thread() {
    if (started_ && !joined_) {
        thread_->detach();
    }
}