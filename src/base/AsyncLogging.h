#ifndef MUDUOZ_BASE_ASYNCLOGGING_H
#define MUDUOZ_BASE_ASYNCLOGGING_H
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <boost/noncopyable.hpp>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "ClangSpec.h"
#include "Thread.h"
namespace muduoZ {
namespace detail {
template <int SIZE>
class FixedBuffer {
   public:
    FixedBuffer()
        : cur(data) {}
    ~FixedBuffer(){};

    // Note: check avail before append
    void append(const char* msg, int len) {
        assert(avail() >= len);
        std::memcpy(cur, msg, len);
        cur += len;
    }
    int len() const { return static_cast<int>(cur - data); }
    int avail() const { return SIZE - len(); }
    void reset() { cur = data; }
    const char* begin() const { return data; }
    char* current() const { return cur; }
    void addCurrent(size_t len) { cur += len; }

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
    FixedBufferWithCookie() { cookie = cookieStart; }
    ~FixedBufferWithCookie() { cookie = cookieEnd; }

   private:
    void (*cookie)();
    static void cookieStart(){};
    static void cookieEnd(){};
};
}  // namespace detail
}  // namespace muduoZ

class AsyncLogging : boost::noncopyable {
   public:
    AsyncLogging() : curBufPtr(new Buffer), thread(std::bind(&AsyncLogging::threadFunc, this)), started_(false) {}
    ~AsyncLogging() {}

    void append(const char* msg, int len);  // front-end
    void threadFunc();                      // back-end
    void start();

   private:
    typedef muduoZ::detail::FixedBufferWithCookie<muduoZ::detail::LargeBufferSize> Buffer;
    typedef std::unique_ptr<Buffer> BufferPtr;
    typedef std::vector<BufferPtr> BufferPtrVector;

    std::mutex mtx;
    std::condition_variable cv;
    BufferPtr curBufPtr;  // NEW THING: I don't think prepare 2 buffer (currentBuffer_ and nextBuffer_ in muduo) is necessary, it makes things more complicated and makes reader being confused about the meaning of "Double Buffering" technique. If we think buffer space is not large enough, just enlarge cur's buffer size(8MB, for example).
    BufferPtrVector filledBuffersPtr;

    const int flushInterval_ = 3;
    const unsigned bufferOverloadThreash_ = 25;  // When there is more than 25 buffers need to flush,
                                                 // we throw away them.(check threadFunc())
    Thread thread;
    bool started_;
};

#endif