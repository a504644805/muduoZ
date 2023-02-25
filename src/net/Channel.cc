#include "Channel.h"

void Channel::handleEvent() {
    if (revents_ & (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI))
        if (onReadableCb_) onReadableCb_();
    if (revents_ & (POLLOUT | POLLWRNORM | POLLWRBAND))
        if (onWriteableCb_) onWriteableCb_();
    if (revents_ & (~capable_))  // TODO:handle other events
        LOG_SYSFATAL << "Don't know how to handle this(these) event(s): " << (revents_ & (~capable_));
}
