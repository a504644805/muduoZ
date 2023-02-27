#ifndef MUDUOZ_NET_CHANNEL_H
#define MUDUOZ_NET_CHANNEL_H
#include <poll.h>

#include <boost/noncopyable.hpp>
#include <functional>

#include "Logger.h"
class Poller;
// 负责某个fd的事件分发
// TODO:目前Channel只支持在Poller中的一次注册和注销。让Channel支持重复注册(If necessary)。
class Channel : public boost::noncopyable {
   public:
    typedef std::function<void()> EventCallback;
    Channel(int fd) : fd_(), events_(0), revents_(0) {}
    ~Channel() {}  // Life circle of fd is responsible for owner of channel (TcpConnection, Acceptor etc)

    void handleEvent();

    int fd() { return fd_; }
    int events() { return events_; }
    int revents() { return revents_; }
    void set_revents(int revents) { revents_ = revents; }
    void set_onReadableCb_(EventCallback cb) { onReadableCb_ = cb; }
    void enableReading() { events_ |= (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI); }

    // helper
    std::string reventsToString() const;
    std::string eventsToString() const;
    std::string eventsToString(int fd, int ev) const;

   private:
    int fd_;
    int events_;
    int revents_;
    EventCallback onReadableCb_;
    EventCallback onWriteableCb_;

    const int capable_ = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI | POLLOUT | POLLWRNORM | POLLWRBAND;  // event we current support
};

#endif