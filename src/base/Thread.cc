#include "Thread.h"

// std::function can not implicitlt converted to function poiter(which is need by pthread_create), hence we need threadFuncHelper.
// However we shouldn't simply transfer std::function* to threadFuncHelper, as we can't gurantee the life circle of parameter, hence we need struct ThreadFuncHelper to make a copy of std::function
struct ThreadFuncHelper {
    typedef Thread::ThreadFunc ThreadFunc;
    ThreadFuncHelper(ThreadFunc func) : function_(func) {}
    ~ThreadFuncHelper() {}
    ThreadFunc function_;
};
void* threadFuncHelper(void* obj) {
    ThreadFuncHelper* helper = static_cast<ThreadFuncHelper*>(obj);
    helper->function_();
    delete helper;
    return static_cast<void*>(0);
}

void Thread::start() {
    ThreadFuncHelper* helper = new ThreadFuncHelper(func_);
    if (int ret = pthread_create(&pthreadId_, NULL, threadFuncHelper, static_cast<void*>(helper))) {
        fprintf(stderr, "Thread::start(Line %d): pthread_create failed at %s, errno=%d\n",
                __LINE__, Timestamp::now().toFormattedString().c_str(), ret);
    } else {
        started_ = true;
    }
}

void Thread::join() {
    assert(started_ && !joined_);  // code logic err
    if (int ret = pthread_join(pthreadId_, NULL)) {
        fprintf(stderr, "Thread::join(Line %d): pthread_join failed at %s, errno=%d\n",
                __LINE__, Timestamp::now().toFormattedString().c_str(), ret);
    } else {
        joined_ = true;
    }
}

Thread::~Thread() {
    if (started_ && !joined_) {
        if (int ret = pthread_detach(pthreadId_))
            fprintf(stderr, "Thread::~Thread(Line %d): pthread_detach failed at %s, errno=%d\n",
                    __LINE__, Timestamp::now().toFormattedString().c_str(), ret);
    }
}