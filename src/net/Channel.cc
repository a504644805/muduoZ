#include "Channel.h"

#include <sstream>
void Channel::handleEvent() {
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        assert(guard);
    }
    auto nowtime = Timestamp::now().toFormattedString();
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {  //??? When will this happen.
        if (onCloseCb_)
            onCloseCb_();
    }
    if (revents_ & (EPOLLIN | EPOLLPRI)) {
        if (onReadableCb_)
            onReadableCb_();
    }
    if (revents_ & EPOLLOUT) {
        if (onWriteableCb_)
            onWriteableCb_();
    }
    if (revents_ & (~capable_))  // TODO:handle other events
        LOG_SYSFATAL << "Don't know how to handle some of this(these) event(s): " << eventsToString(fd_, revents_).c_str();
}

// copy from muduo
std::string Channel::reventsToString() const {
    return eventsToString(fd_, revents_);
}
std::string Channel::eventsToString() const {
    return eventsToString(fd_, events_);
}
std::string Channel::eventsToString(int fd, int ev) const {
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & EPOLLIN)
        oss << "IN ";
    if (ev & EPOLLOUT)
        oss << "OUT ";
    if (ev & EPOLLRDHUP)
        oss << "RDHUP ";
    if (ev & EPOLLPRI)
        oss << "PRI ";
    if (ev & EPOLLERR)
        oss << "ERR ";
    if (ev & EPOLLHUP)
        oss << "HUP ";
    if (ev & EPOLLET)
        oss << "ET ";
    if (ev & EPOLLONESHOT)
        oss << "ONESHOT ";
    if (ev & EPOLLWAKEUP)
        oss << "WAKEUP ";
    if (ev & EPOLLEXCLUSIVE)
        oss << "EXCLUSIVE ";
    if (int unknownEv = ev & (~(EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLET | EPOLLONESHOT | EPOLLWAKEUP | EPOLLEXCLUSIVE))) {
        LOG_SYSFATAL << "Unknown events: " << unknownEv << " known events: " << oss.str().c_str();
    }
    return oss.str();
}

void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}