#ifndef MUDUOZ_SRC_BASE_THREAD_H
#define MUDUOZ_SRC_BASE_THREAD_H

#include <assert.h>
#include <pthread.h>

#include <functional>

#include "Timestamp.h"
#include "noncopyable.h"
class Thread : muduoZ::noncopyable {
   public:
    typedef std::function<void()> ThreadFunc;  // we don't use void*(void*) as threadFunc type. User can use "function + bind" to pass argument and transfer result.
    explicit Thread(ThreadFunc func) : func_(func), started_(false), joined_(false) {
    }
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
    pthread_t pthreadId_;
};

#endif