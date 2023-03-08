#ifndef MUDUOZ_SRC_BASE_THREAD_H
#define MUDUOZ_SRC_BASE_THREAD_H

#include <assert.h>
#include <pthread.h>

#include <functional>
#include <thread>

#include "CountDownLatch.h"
#include "Timestamp.h"
#include "noncopyable.h"
class Thread : muduoZ::noncopyable {
   public:
    typedef std::function<void()> ThreadFunc;  // we don't use void*(void*) as threadFunc type. User can use "function + bind" to pass argument and transfer result.
    explicit Thread(ThreadFunc func) : func_(func), started_(false), joined_(false), pthreadId_(0), tid_(0), latch_(1) {}
    ~Thread();
    /*Usage:
        Thread thread(func,arg);
        thread.start()
        thread.join() //optional
    */
    void start();
    void join();

   private:
    ThreadFunc func_;
    bool started_;
    bool joined_;
    std::thread::id pthreadId_;
    pid_t tid_;  // unique identification of a thread

    std::unique_ptr<std::thread> thread_;

    CountDownLatch latch_;
};

#endif