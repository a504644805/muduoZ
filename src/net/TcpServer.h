#ifndef MUDUOZ_SRC_NET_TCPSERVER_H
#define MUDUOZ_SRC_NET_TCPSERVER_H

#include <functional>
#include <memory>
#include <unordered_set>

#include "Acceptor.h"
#include "callback.h"
#include "noncopyable.h"
class TcpConnection;
class Buffer;
class EventLoop;
class TcpServer : muduoZ::noncopyable {
   public:
    TcpServer(SockAddrIn& bindAddr, EventLoop* loop);
    ~TcpServer();
    void start();

    void set_onConnectionCb(const OnConnectionCb& cb) { onConnectionCb_ = cb; }
    void set_onCloseCb(const OnCloseCb& cb) { onCloseCb_ = cb; }
    void set_onMessageCb(const OnMessageCb& cb) { onMessageCb_ = cb; }
    void set_onWriteCompleteCb(const OnWriteCompleteCb& cb) { onWriteCompleteCb_ = cb; }

    // when a connection is created or closed, there are something need to be done by TcpServer.
    void newConnection(int connfd);                       // called by Acceptor when new conn arrives.
    void removeConnection(const TcpConnectionPtr& conn);  // called by TcpConnection when a conn is closed

   private:
    Acceptor acceptor_;
    std::unordered_set<TcpConnectionPtr> connectionSet_;

    OnConnectionCb onConnectionCb_;
    OnCloseCb onCloseCb_;
    OnMessageCb onMessageCb_;
    OnWriteCompleteCb onWriteCompleteCb_;

    EventLoop* loop_;
};

#endif