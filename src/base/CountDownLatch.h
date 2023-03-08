#ifndef MUDUOZ_COUNTDOWNLATCH_H
#define MUDUOZ_COUNTDOWNLATCH_H

#include <condition_variable>
#include <mutex>

#include "noncopyable.h"

class CountDownLatch : muduoZ::noncopyable {
   public:
    explicit CountDownLatch(int cnt) : cnt_(cnt) {}

    void wait();

    void countDown();

   private:
    int cnt_;                     // protected by mtx_
    std::condition_variable cv_;  // protected by mtx_
    std::mutex mtx_;
};

#endif