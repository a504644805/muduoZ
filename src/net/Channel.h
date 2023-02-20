#ifndef MUDUOZ_NET_CHANNEL_H
#define MUDUOZ_NET_CHANNEL_H
#include <poll.h>
#include <functional>
// 关注某个fd，进行事件分发
class Channel {
   public:
    typedef std::function<void()> EventCallback;
    Channel(int fd) : fd_() {}
    ~Channel() {}  // Life circle of fd is responsible for TcpConnection

    void handleEvent() {
        if (revents_ & (POLLIN | POLLRDNORM))
            onReadableCb_();
        else if (revents_ & (POLLOUT | POLLWRNORM))
            onWriteableCb_();
        else
            ;
        // LOG_FATAL << "Unknown event hannpend";
        // TODO:handle other events
    }

   private:
    int fd_;
    int events_;
    int revents_;
    EventCallback onReadableCb_;
    EventCallback onWriteableCb_;
};

#endif