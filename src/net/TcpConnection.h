#ifndef MUDUOZ_SRC_NET_TCPCONNECTION_H
#define MUDUOZ_SRC_NET_TCPCONNECTION_H

#include <boost/noncopyable.hpp>
#include <memory>

#include "Buffer.h"
#include "callback.h"
class Channel;
class Socket;
class EventLoop;
class TcpConnection : public boost::noncopyable, public std::enable_shared_from_this<TcpConnection> {
   public:
    typedef std::function<void(const TcpConnectionPtr&)> CbForTcpServer_onClose;
    TcpConnection(int sockfd, EventLoop* loop);
    ~TcpConnection();

    void connEstablished();  // called by TcpServer
    void handleRead();
    void handleWrite();
    void handleClose();

    void send(const char* str, int len);

    void set_OnConnectionCb(const OnConnectionCb& cb) { onConnectionCb = cb; }
    void set_OnCloseCb(const OnCloseCb& cb) { onCloseCb = cb; }
    void set_OnMessageCb(const OnMessageCb& cb) { onMessageCb = cb; }
    void set_OnWriteCompleteCb(const OnWriteCompleteCb& cb) { onWriteCompleteCb = cb; }

    void set_CbForTcpServer_onClose(const CbForTcpServer_onClose& cb) { cbForTcpServer_onClose = cb; }

    void setTcpNoDelay(bool on);

   private:
    std::unique_ptr<Channel> channel_;  // use unique_ptr instead of Channel channel_ so we don't need to include "Channel.h"
    Buffer inputBuffer;
    Buffer outputBuffer;

    std::unique_ptr<Socket> socket_;  // for simplity of using socket operation such as read/write
    EventLoop* loop_;

    OnConnectionCb onConnectionCb;
    OnCloseCb onCloseCb;
    OnMessageCb onMessageCb;
    OnWriteCompleteCb onWriteCompleteCb;

    CbForTcpServer_onClose cbForTcpServer_onClose;
};

#endif