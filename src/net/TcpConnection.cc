#include "TcpConnection.h"

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
TcpConnection::TcpConnection(int sockfd, EventLoop* loop) : channel_(new Channel(sockfd)), socket_(new Socket(sockfd)), loop_(loop) {
}
TcpConnection::~TcpConnection() {
    loop_->removeChannel(channel_.get());
    // destroy socket RAII
}

void TcpConnection::connEstablished() {  // called by TcpServer
    channel_->set_onReadableCb_(std::bind(&TcpConnection::handleRead, this));
    channel_->set_onWriteableCb_(std::bind(&TcpConnection::handleWrite, this));
    channel_->set_onCloseCb_(std::bind(&TcpConnection::handleClose, this));
    channel_->enableReading();  // Don't enable writing here. If we don't send, POLLOUT will always be here (under Level triger)
    loop_->updateChannel(channel_.get());

    assert(onConnectionCb);
    onConnectionCb(shared_from_this());
}

void TcpConnection::handleRead() {
    char buf[65536];
    int len = socket_->read(buf, sizeof buf);
    if (len == 0) {
        handleClose();
    } else {
        assert(onMessageCb);
        onMessageCb(shared_from_this(), buf, len);
    }
}
void TcpConnection::handleWrite() {
    /*
    if output buffer is not empty
        write as much as we can
    else
        disable writing
    */
}
void TcpConnection::handleClose() {
    loop_->removeChannel(channel_.get());
    onCloseCb(shared_from_this());               // L1
    cbForTcpServer_onClose(shared_from_this());  // L2. sequence of L1 and L2 should't be switched, Or TcpConnection may be destroy before onCloseCb
}

void TcpConnection::send(const char* str, int len) {
}