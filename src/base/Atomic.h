// copy from muduo
#ifndef MUDUOZ_BASE_ATOMIC_H
#define MUDUOZ_BASE_ATOMIC_H

#include <stdint.h>

namespace muduoZ {
namespace detail {

template <typename T>
class AtomicIntegerT {
   public:
    AtomicIntegerT()
        : value_(0) {
    }

    T getAndAdd(T x) {
        // in gcc >= 4.7: __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST)
        return __sync_fetch_and_add(&value_, x);
    }

    T addAndGet(T x) {
        return getAndAdd(x) + x;
    }

    T incrementAndGet() {
        return addAndGet(1);
    }

    T decrementAndGet() {
        return addAndGet(-1);
    }

   private:
    volatile T value_;
};
}  // namespace detail
typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
typedef detail::AtomicIntegerT<int64_t> AtomicInt64;
}  // namespace muduoZ

#endif  // MUDUO_BASE_ATOMIC_H
