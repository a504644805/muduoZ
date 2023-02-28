#include "TcpServer.h"

TcpServer::TcpServer(SockAddrIn& bindAddr, EventLoop* loop) : acceptor_(bindAddr, loop), loop_(loop) {
    acceptor_.set_newConnectionCb(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
}

// TODO: close all connections ?
TcpServer::~TcpServer() {}

void TcpServer::start() { acceptor_.start(); }
void TcpServer::newConnection(int connfd) {
    /*
        new and configure a TcpConnection and update connectionSet_
        call onConnectionCb_
    */
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    /*
        remove the Connection from connectionSet
        call onCloseCb_
    */
}