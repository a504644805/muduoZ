#include "CountDownLatch.h"

void CountDownLatch::wait() {
    std::unique_lock<std::mutex> lk(mtx_);
    while (cnt_ > 0) {
        cv_.wait(lk);
    }
}

void CountDownLatch::countDown() {
    std::lock_guard<std::mutex> guard(mtx_);
    --cnt_;
    if (cnt_ == 0) {
        cv_.notify_all();
    }
}