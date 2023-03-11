#include "AsyncLogging.h"

#include "Timestamp.h"

AsyncLogging g_asynclogging;

// 前端不停往cur写数据，直到cur写满。把cur放入filledBuffer并new一个新Buffer给cur
void AsyncLogging::append(const char* msg, int len) {
    std::lock_guard<std::mutex> guard(mtx);
    if (curBufPtr->avail() >= len)
        curBufPtr->append(msg, len);
    else {
        filledBuffersPtr.push_back(std::move(curBufPtr));
        curBufPtr.reset(new Buffer);
        curBufPtr->append(msg, len);
        cv.notify_one();
    }
}

// 后端定时定量地取数据：把cur和filledBuffers中的Buffer指针取走（不复制内存，减少cs）
void AsyncLogging::threadFunc() {
    std::string filename = "Log." + Timestamp::now().toFormattedString();
    int fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_APPEND | O_EXCL | O_CLOEXEC, S_IRUSR | S_IWRITE);
    if (fd < 0) {
        fprintf(stderr, "AsyncLogging::threadFunc(Line %d): open failed, errno = %d\n", __LINE__, errno);
        assert(fd >= 0);  // FIXME:terminate the process.(now is Debug version, we simply use assert to terminate)
    }

    BufferPtr newBufPtr(new Buffer());
    BufferPtrVector buffersToWrite;
    while (!quit_) {
        {
            std::unique_lock<std::mutex> lk(mtx);
            if (filledBuffersPtr.empty())
                cv.wait_for(lk, std::chrono::milliseconds(flushInterval_ * 1000));
            buffersToWrite.swap(filledBuffersPtr);
            buffersToWrite.push_back(std::move(curBufPtr));
            curBufPtr = std::move(newBufPtr);
            curBufPtr->reset();
        }
        if (buffersToWrite.size() > bufferOverloadThreash_) {
            fprintf(stderr, "Dropped log messages at %s, drop %zd buffers\n",
                    Timestamp::now().toFormattedString().c_str(),
                    buffersToWrite.size() - bufferOverloadThreash_);
            buffersToWrite.erase(buffersToWrite.begin() + bufferOverloadThreash_, buffersToWrite.end());
        }
        // writev (gather write into page cache)
        int iovcnt = static_cast<int>(buffersToWrite.size());
        struct iovec vec[bufferOverloadThreash_];
        int totalBytesToWrite = 0;
        for (int i = 0; i < iovcnt; i++) {
            vec[i].iov_base = const_cast<char*>(buffersToWrite[i]->begin());
            int availBytes = buffersToWrite[i]->len();
            vec[i].iov_len = availBytes;
            totalBytesToWrite += availBytes;
        }
        const ssize_t n = writev(fd, vec, iovcnt);
        if (n < 0) {
            fprintf(stderr, "AsyncLogging::threadFunc:(Line %d): writev failed at %s, errno = %d\n", __LINE__, Timestamp::now().toFormattedString().c_str(), errno);
        } else if (n != totalBytesToWrite) {
            fprintf(stderr, "AsyncLogging::threadFunc:(Line %d): n(%zd) != totalBytesToWrite(%d) at %s\n", __LINE__, n, totalBytesToWrite, Timestamp::now().toFormattedString().c_str());
        }

        assert(!buffersToWrite.empty());
        newBufPtr = std::move(buffersToWrite[0]);
        newBufPtr->reset();
        buffersToWrite.clear();  // writev has flush data into page cache, now we can destroy buffers.
                                 // if program dump before destroy buffers here, we can search cookieStart in core file
    }
    // close(fd); close by OS when process terminated.
}

void AsyncLogging::start() {
    if (!started_) {
        thread.start();
        started_ = true;
    }
}

void AsyncLogging::stop() {
    quit_ = true;
    cv.notify_one();
    thread.join();
}