#include "LogStream.h"

#include <string.h>

#include <algorithm>
namespace muduoZ {
namespace detail {

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
// Efficient Integer to String Conversions, by Matthew Wilson.
template <typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

#define kMaxIntStrLength 11

}  // namespace detail
}  // namespace muduoZ

LogStream& LogStream::operator<<(int v) {
    if (buffer.avail() >= kMaxIntStrLength) {
        size_t len = muduoZ::detail::convert(buffer.current(), v);
        buffer.addCurrent(len);
    } else {
        overLen = true;
    }
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v) {
    if (buffer.avail() >= kMaxIntStrLength) {
        size_t len = muduoZ::detail::convert(buffer.current(), v);
        buffer.addCurrent(len);
    } else {
        overLen = true;
    }
    return *this;
}

LogStream& LogStream::operator<<(const char* v) {
    if (buffer.avail() >= static_cast<int>(strlen(v))) {
        buffer.append(v, static_cast<int>(strlen(v)));
    } else {
        overLen = true;
    }
    return *this;
}