#include "TcpServer.h"

#include "TcpConnection.h"
TcpServer::TcpServer(SockAddrIn& bindAddr, EventLoop* loop) : acceptor_(bindAddr, loop), loop_(loop) {
    acceptor_.set_newConnectionCb(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
}

// TODO: close all connections ?
TcpServer::~TcpServer() {}

void TcpServer::start() { acceptor_.start(); }
void TcpServer::newConnection(int connfd) {
    assert(onConnectionCb_);
    assert(onCloseCb_);
    assert(onMessageCb_);
    assert(onWriteCompleteCb_);
    /*
        new and configure a TcpConnection and update connectionSet_
    */
    TcpConnectionPtr conn(new TcpConnection(connfd, loop_));
    conn->set_OnConnectionCb(onConnectionCb_);
    conn->set_OnCloseCb(onCloseCb_);
    conn->set_OnMessageCb(onMessageCb_);
    conn->set_OnWriteCompleteCb(onWriteCompleteCb_);
    conn->set_CbForTcpServer_onClose(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    connectionSet_.insert(conn);
    conn->connEstablished();
}

// FIXME:Only support called by TcpConnection now. Don't actively call this function
void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    /*
        remove the Connection from connectionSet
    */
    auto iter = connectionSet_.find(conn);
    assert(iter != connectionSet_.end());
    connectionSet_.erase(iter);
}