#include "TimerQueue.h"

#include <string.h>
#include <sys/timerfd.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

namespace muduoZ {
namespace detail {
// copy from muduo
struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.microseconds() - Timestamp::now().microseconds();
    if (microseconds < 100) {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microseconds / Timestamp::kMicrosecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
        (microseconds % Timestamp::kMicrosecondsPerSecond) * 1000);
    return ts;
}
void readTimerfd(int timerfd, Timestamp now) {
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    if (n != sizeof howmany) {
        LOG_ERROR << "TimerQueue::handleRead() reads " << static_cast<int>(n) << " bytes instead of 8";  // FIXME:remove static_cast
    }
}
void resetTimerfd(int timerfd, Timestamp expiration) {
    // wake up loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof newValue);
    memset(&oldValue, 0, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        LOG_SYSFATAL << "timerfd_settime()";
    }
}
}  // namespace detail
}  // namespace muduoZ

TimerQueue::TimerQueue(EventLoop* loop) : loop_(loop) {
    timerfd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd_ < 0)
        LOG_SYSFATAL << "timerfd_create failed";
    timerfd_channel_.reset(new Channel(timerfd_));
    timerfd_channel_->set_onReadableCb_(std::bind(&TimerQueue::handleRead, this));
    timerfd_channel_->enableReading();
    loop_->updateChannel(timerfd_channel_.get());
}

// Same life circle as EventLoop
TimerQueue::~TimerQueue() {
    ::close(timerfd_);
    // handle timerfd_channel_: let Channel's destructor for its destruction. No need to logout from Poller, because EventLoop is dying
    for (const Entry& entry : timerSet_)
        delete entry.second;
}

void TimerQueue::handleRead() {
    Timestamp now(Timestamp::now());
    muduoZ::detail::readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);
    for (const Entry& it : expired) {
        it.second->runCb();
    }

    for (const Entry& it : expired) {
        delete it.second;
    }
    if (!timerSet_.empty()) {
        Timestamp nextExpire = timerSet_.begin()->first;
        muduoZ::detail::resetTimerfd(timerfd_, nextExpire);
    }
}
// MOVE expired entries to a local vector and return this vector
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    std::vector<Entry> expired;
    Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));

    TimerSet::iterator end = timerSet_.lower_bound(sentry);  // find first entry whose expire_time > now
    assert(end == timerSet_.end() || now < end->first);
    std::copy(timerSet_.begin(), end, back_inserter(expired));
    timerSet_.erase(timerSet_.begin(), end);

    return expired;
}

void TimerQueue::addTimer(const Timer::TimerCallback& cb, Timestamp expiration_time) {
    bool earlistExpirationTimeWillChange = false;
    if (timerSet_.empty() || expiration_time < timerSet_.begin()->first)
        earlistExpirationTimeWillChange = true;

    Timer* timer = new Timer(cb, expiration_time);
    assert(timerSet_.insert({expiration_time, timer}).second == true);

    if (earlistExpirationTimeWillChange) {
        assert(timerSet_.begin()->first == expiration_time);
        muduoZ::detail::resetTimerfd(timerfd_, expiration_time);
    }
}