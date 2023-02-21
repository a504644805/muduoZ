#include "AsyncLogging.h"
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
    BufferPtr newBufPtr(new Buffer());
    BufferPtrVector buffersToWrite;
    while (1) {
        {
            std::unique_lock<std::mutex> lk(mtx);
            if (filledBuffersPtr.empty())
                cv.wait_for(lk, std::chrono::milliseconds(flushInterval_ * 1000));
            buffersToWrite.swap(filledBuffersPtr);
            buffersToWrite.push_back(std::move(curBufPtr));
            curBufPtr = std::move(newBufPtr);
            curBufPtr->reset();
        }
        // flush;
        assert(!buffersToWrite.empty());
        newBufPtr = std::move(buffersToWrite[0]);
        newBufPtr->reset();
        buffersToWrite.clear();
    }
}