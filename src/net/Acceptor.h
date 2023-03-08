#ifndef MUDUOZ_SRC_NET_ACCEPTOR_H
#define MUDUOZ_SRC_NET_ACCEPTOR_H

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "Channel.h"
#include "Socket.h"
#include "noncopyable.h"
class EventLoop;
class Acceptor : muduoZ::noncopyable {
   public:
    typedef std::function<void(int)> NewConnectionCb;

    Acceptor(SockAddrIn& bindAddr, EventLoop* loop);
    ~Acceptor();
    void start();
    void handleRead();

    void set_newConnectionCb(NewConnectionCb&& cb) { newConnectionCb = cb; }

   private:
    Socket acceptSocket_;
    Channel acceptChannel_;
    EventLoop* loop_;

    NewConnectionCb newConnectionCb;
};

#endif