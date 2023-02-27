#ifndef MUDUOZ_SRC_NET_TCPSERVER_H
#define MUDUOZ_SRC_NET_TCPSERVER_H

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <unordered_set>

#include "Acceptor.h"
class TcpConnection;
class TcpServer : public boost::noncopyable {
   public:
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

    TcpServer(SockAddrIn& bindAddr, EventLoop* loop) : acceptor_(bindAddr, loop) {
        acceptor_.set_newConnectionCb(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
    }
    ~TcpServer();
    void start() { acceptor_.start(); }
    void newConnection(int connfd);
    // a series cb interface for user

   private:
    Acceptor acceptor_;
    std::unordered_set<TcpConnectionPtr> connectionSet_;
};

#endif