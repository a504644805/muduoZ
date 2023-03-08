#ifndef MUDUOZ_SRC_NET_TCPCLIENT_H
#define MUDUOZ_SRC_NET_TCPCLIENT_H

#include <memory>

#include "callback.h"
#include "noncopyable.h"
class Connector;
class EventLoop;
class SockAddrIn;
// TODO: support retry after disconnect
class TcpClient : muduoZ::noncopyable {
   public:
    TcpClient(const SockAddrIn& serverAddr, EventLoop* loop);
    ~TcpClient();
    void start();

    void set_onConnectionCb(const OnConnectionCb& cb) { onConnectionCb_ = cb; }
    void set_onCloseCb(const OnCloseCb& cb) { onCloseCb_ = cb; }
    void set_onMessageCb(const OnMessageCb& cb) { onMessageCb_ = cb; }
    void set_onWriteCompleteCb(const OnWriteCompleteCb& cb) { onWriteCompleteCb_ = cb; }

    void newConnection(int connfd);
    void removeConnection(const TcpConnectionPtr& conn);

    void disconnect();

   private:
    std::unique_ptr<Connector> connector_;
    TcpConnectionPtr connection_;

    OnConnectionCb onConnectionCb_;
    OnCloseCb onCloseCb_;
    OnMessageCb onMessageCb_;
    OnWriteCompleteCb onWriteCompleteCb_;

    EventLoop* loop_;
};

#endif