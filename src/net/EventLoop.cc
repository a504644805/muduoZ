#include "EventLoop.h"

#include <sys/eventfd.h>

#include "Channel.h"
#include "Logger.h"
#include "TimerQueue.h"

namespace {
int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}
}  // namespace

EventLoop::EventLoop()
    : threadId_(::CurrentThread::getTid()), timerQueue_(new TimerQueue(this)), wakeupFd_(createEventfd()), wakeupChannel_(new Channel(wakeupFd_)), callingPendingFunctors_(false), quit_(false) {
    wakeupChannel_->set_onReadableCb_(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
    updateChannel(wakeupChannel_.get());
}

EventLoop::~EventLoop() {}

void EventLoop::loop() {
    LOG_INFO << "loop start running";
    while (!quit_) {
        /*
        poll
        activeChannels[]->handleEvent
        */
        poller_.poll(activeChannels_, kPollTimeMs);
        if (Logger::getLogLevel() <= Logger::TRACE) {
            printActiveChannels();
        }
        for (Channel* channel : activeChannels_) {
            channel->handleEvent();
        }
        doPendingFunctors();
    }
}

// copy from muduo
void EventLoop::printActiveChannels() const {
    for (const Channel* channel : activeChannels_) {
        LOG_TRACE << "{" << channel->reventsToString().c_str() << "} ";
    }
}

// copy from muduo
void EventLoop::runAt(Timestamp time, Timer::TimerCallback cb) {
    timerQueue_->addTimer(cb, time);
}
void EventLoop::runAfter(double delay, Timer::TimerCallback cb) {
    Timestamp time(addTime(Timestamp::now(), delay));
    runAt(time, std::move(cb));
}

// copy from muduo
void EventLoop::runInLoop(Functor cb) {
    if (inThreadId_Thread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}
void EventLoop::queueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> guard(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    if (!inThreadId_Thread() || callingPendingFunctors_) {
        wakeup();
    }
}
void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::wakeup() writes " << static_cast<int>(n) << " bytes instead of 8";
    }
}
void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::handleRead() reads " << static_cast<int>(n) << " bytes instead of 8";
    }
}
void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor& functor : functors) {
        functor();
    }
    callingPendingFunctors_ = false;
}