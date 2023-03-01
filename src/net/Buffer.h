#ifndef MUDUOZ_SRC_NET_BUFFER_H
#define MUDUOZ_SRC_NET_BUFFER_H

#include <assert.h>

#include <vector>
class Buffer {
   public:
    const int DATA_INIT_SIZE = 1024;
    Buffer() : data_(DATA_INIT_SIZE), readIdx_(0), writeIdx_(0) {}
    ~Buffer() {}

    int readFd(int fd);
    void append(const char* buf, int len);

    int readableBytes() { return writeIdx_ - readIdx_; }
    int writeableBytes() { return static_cast<int>(data_.size()) - writeIdx_; }
    int prependableBytes() const { return readIdx_; }
    char* begin() { return &*data_.begin(); }
    char* readPtr() { return begin() + readIdx_; }
    char* writePtr() { return begin() + writeIdx_; }

    void ensureWritableBytes(int len) {
        if (writeableBytes() < len) {
            makeSpace(len);
        }
        assert(writeableBytes() >= len);
    }

    void makeSpace(int len) {
        if (writeableBytes() + prependableBytes() < len) {
            // FIXME: move readable data
            data_.resize(writeIdx_ + len);
        } else {
            // move readable data to the front, make space inside buffer
            int readable = readableBytes();
            std::copy(begin() + readIdx_, begin() + writeIdx_, begin());
            readIdx_ = 0;
            writeIdx_ = readIdx_ + readable;
            assert(readable == readableBytes());
        }
    }

    void retrieve(int len) {
        assert(len <= readableBytes());
        if (len < readableBytes()) {
            readIdx_ += len;
        } else {
            retrieveAll();
        }
    }

    void retrieveAll() {
        readIdx_ = 0;
        writeIdx_ = 0;
    }

   private:
    std::vector<char> data_;
    int readIdx_;
    int writeIdx_;
};

#endif