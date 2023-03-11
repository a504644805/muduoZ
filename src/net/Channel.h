#ifndef MUDUOZ_NET_CHANNEL_H
#define MUDUOZ_NET_CHANNEL_H
#include <poll.h>
#include <sys/epoll.h>

#include <functional>

#include "Logger.h"
#include "noncopyable.h"
class Poller;
// 负责某个fd的事件分发
// TODO:目前Channel只支持在Poller中的一次注册和注销。让Channel支持重复注册(If necessary)。
class Channel : muduoZ::noncopyable {
   public:
    typedef std::function<void()> EventCallback;
    Channel(int fd) : fd_(fd), events_(0), revents_(0), tied_(0) {}
    ~Channel() {}  // Life circle of fd and logout from poller is responsible for owner of channel (TcpConnection, Acceptor etc)

    void handleEvent();

    int fd() { return fd_; }
    int events() { return events_; }
    int revents() { return revents_; }
    void set_revents(int revents) { revents_ = revents; }
    void set_onReadableCb_(EventCallback cb) { onReadableCb_ = cb; }
    void set_onWriteableCb_(EventCallback cb) { onWriteableCb_ = cb; }
    void set_onCloseCb_(EventCallback cb) { onCloseCb_ = cb; }

    void enableReading() { events_ |= (EPOLLIN | EPOLLPRI); }
    void enableWriting() { events_ |= (EPOLLOUT); }
    void disableWriting() { events_ &= ~(EPOLLOUT); }

    // helper
    std::string reventsToString() const;
    std::string eventsToString() const;
    std::string eventsToString(int fd, int ev) const;

    // copy from muduo
    /// Tie this channel to the owner object managed by shared_ptr,
    /// prevent the owner object being destroyed in handleEvent.
    void tie(const std::shared_ptr<void>&);

    bool isWriteing() { return events_ & EPOLLOUT; }

   private:
    int fd_;
    int events_;
    int revents_;
    EventCallback onReadableCb_;
    EventCallback onWriteableCb_;
    EventCallback onCloseCb_;

    const int capable_ = EPOLLIN | EPOLLPRI | EPOLLOUT | EPOLLHUP;  // event we current support

    std::weak_ptr<void> tie_;
    bool tied_;
};

#endif