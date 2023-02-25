#ifndef MUDUOZ_NET_CHANNEL_H
#define MUDUOZ_NET_CHANNEL_H
#include <poll.h>

#include <functional>

#include "Logger.h"
// 负责某个fd的事件分发
class Channel {
   public:
    typedef std::function<void()> EventCallback;
    Channel(int fd) : fd_(), events_(0), revents_(0) {}
    ~Channel() {}  // Life circle of fd is responsible for owner of channel (TcpConnection, Acceptor etc)

    void handleEvent();

   private:
    int fd_;
    int events_;
    int revents_;
    EventCallback onReadableCb_;
    EventCallback onWriteableCb_;

    const int capable_ = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI | POLLOUT | POLLWRNORM | POLLWRBAND;  // event we current support
};

#endif