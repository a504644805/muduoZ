#ifndef MUDUOZ_SRC_BASE_LOGSTREAM_H
#define MUDUOZ_SRC_BASE_LOGSTREAM_H

#include <boost/noncopyable.hpp>

#include "AsyncLogging.h"
class LogStream : boost::noncopyable {
   public:
    LogStream() : overLen(false) {}
    ~LogStream() {}
    typedef muduoZ::detail::FixedBufferWithCookie<muduoZ::detail::SmallBufferSize> Buffer;
    typedef LogStream self;
    self& operator<<(int v);
    self& operator<<(unsigned int v);
    self& operator<<(const char* v);

    const Buffer& getBuffer() { return buffer; }
    bool getOverLen() { return overLen; }

   private:
    Buffer buffer;

    bool overLen;
};

#endif