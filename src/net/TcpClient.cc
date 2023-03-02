
#include "TcpClient.h"

#include <assert.h>

#include "Connector.h"
#include "TcpConnection.h"

TcpClient::TcpClient(const SockAddrIn& serverAddr, EventLoop* loop)
    : connector_(new Connector(serverAddr, loop)), loop_(loop) {
    connector_->set_newConnecitonCb(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

// TODO:close the connection (if it's connected)
TcpClient::~TcpClient() {
}

void TcpClient::start() {
    connector_->start();
}

void TcpClient::newConnection(int connfd) {
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
    conn->set_CbForTcpServer_onClose(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    connection_ = conn;
    conn->connEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn) {
    connection_.reset();
}
