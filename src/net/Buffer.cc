#include "Buffer.h"

#include <sys/uio.h>

#include "Logger.h"

// copy from muduo
int Buffer::readFd(int sockfd) {
    char extrabuf[65536];
    struct iovec vec[2];
    const int writable = writeableBytes();
    vec[0].iov_base = begin() + writeIdx_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // when there is enough space in this buffer, don't read into extrabuf.
    // when extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writable < static_cast<int>(sizeof extrabuf)) ? 2 : 1;
    const int n = static_cast<int>(::readv(sockfd, vec, iovcnt));
    if (n < 0) {
        LOG_SYSFATAL << "read failed";
    } else if (n <= writable) {
        writeIdx_ += n;
    } else {
        writeIdx_ = static_cast<int>(data_.size());
        append(extrabuf, static_cast<int>(n - writable));
    }
    return n;
}

void Buffer::append(const char* buf, int len) {
    ensureWritableBytes(len);
    std::copy(buf, buf + len, writePtr());
    writeIdx_ += len;
}