#ifndef MUDUOZ_BASE_ASYNCLOGGING_H
#define MUDUOZ_BASE_ASYNCLOGGING_H
#include <assert.h>
#include <boost/noncopyable.hpp>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>
#include "ClangSpec.h"
namespace muduoZ {
namespace detail {
template <int SIZE>
class FixedBuffer {
   public:
    FixedBuffer()
        : cur(data) {}
    ~FixedBuffer();

    // Note: check avail before append
    void append(const char* msg, int len) {
        assert(avail >= len);
        std::memcpy(cur, msg, len);
        cur += len;
    }
    int len() const { return implicit_cast<int>(cur - data); }
    int avail() const {return SIZE - len()};

   private:
    char data[SIZE];
    char* cur;
};

// Just for Logging
static const int LargeBufferSize = 4000 * 1000;  // 4MB, used for AsyncLogging
static const int SmallBufferSize = 4000;         // 4KB, used for LogStream
template <int SIZE>
class FixedBufferWithCookie
    : public muduoZ::detail::FixedBuffer<
          SIZE> {  // NEW THING: We don't put cookie_ inside
                   // FixedBuffer, so others can
                   // freely use FixedBuffer without having
                   // same cookie as AsyncLogging, which may
                   // confuse us
   public:
    Buffer() { cookie = cookieStart; }
    ~Buffer() { cookie = cookieEnd; }

   private:
    void (*cookie)();
    static void cookieStart(){};
    static void cookieEnd(){};
};
}  // namespace detail
}  // namespace muduoZ

class AsyncLogging : boost::noncopyable {
   public:
    void append(const char* msg, int len);  // front-end
    void threadFunc();                      // back-end

   private:
    typedef muduoZ::detail::FixedBufferWithCookie<muduoZ::detail::LargeBufferSize> Buffer;
    typedef std::unique_ptr<Buffer> BufferPtr;
    typedef std::vector<BufferPtr> BufferPtrVector;

    // std::mutex();
    BufferPtr cur;  // NEW THING: I don't think prepare 2 buffer (currentBuffer_ and nextBuffer_ in muduo) is necessary, it makes things more complicated and makes reader being confused about the meaning of "Double Buffering" technique. If we think buffer space is not large enough, just enlarge cur's buffer size(8MB, for example).
    BufferPtrVector filledBuffers;
};

#endif