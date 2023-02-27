#include "Channel.h"

#include <sstream>
void Channel::handleEvent() {
    if (revents_ & (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI))
        if (onReadableCb_) onReadableCb_();
    if (revents_ & (POLLOUT | POLLWRNORM | POLLWRBAND))
        if (onWriteableCb_) onWriteableCb_();
    if (revents_ & (~capable_))  // TODO:handle other events
        LOG_SYSFATAL << "Don't know how to handle this(these) event(s): " << (revents_ & (~capable_));
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
    if (ev & POLLIN)
        oss << "IN ";
    if (ev & POLLPRI)
        oss << "PRI ";
    if (ev & POLLOUT)
        oss << "OUT ";
    if (ev & POLLHUP)
        oss << "HUP ";
    if (ev & POLLRDHUP)
        oss << "RDHUP ";
    if (ev & POLLERR)
        oss << "ERR ";
    if (ev & POLLNVAL)
        oss << "NVAL ";

    return oss.str();
}