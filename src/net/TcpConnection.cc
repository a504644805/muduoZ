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

void TcpConnection::connEstablished() {  // called by TcpServer::newConnection
    channel_->set_onReadableCb_(std::bind(&TcpConnection::handleRead, this));
    channel_->set_onWriteableCb_(std::bind(&TcpConnection::handleWrite, this));
    channel_->set_onCloseCb_(std::bind(&TcpConnection::handleClose, this));
    channel_->enableReading();  // Don't enable writing here. If we don't send, POLLOUT will always be here (under Level triger)
    channel_->tie(shared_from_this());
    loop_->updateChannel(channel_.get());

    assert(onConnectionCb);
    onConnectionCb(shared_from_this());
}

void TcpConnection::handleRead() {
    int len = inputBuffer.readFd(socket_->sockfd());
    if (len == 0) {
        handleClose();
    } else {
        assert(onMessageCb);
        onMessageCb(shared_from_this(), &inputBuffer);
    }
}
void TcpConnection::handleWrite() {
    /*
    if outputBuffer is not empty
        write as much as we can
        if outputBuffer is empty, writeCompleteCb
    else
        disable writing
    */
    if (outputBuffer.readableBytes() > 0) {
        int n = socket_->write(outputBuffer.readPtr(), outputBuffer.readableBytes());
        outputBuffer.retrieve(n);
        if (outputBuffer.writeableBytes() == 0) {
            if (onWriteCompleteCb)
                onWriteCompleteCb(shared_from_this());
        }
    }
    if (outputBuffer.readableBytes() == 0) {
        channel_->disableWriting();  // avoid POLLOUT (because we are level trigger)
        loop_->updateChannel(channel_.get());
    }
}
void TcpConnection::handleClose() {
    loop_->removeChannel(channel_.get());
    onCloseCb(shared_from_this());
    cbForTcpServer_onClose(shared_from_this());  //
}

void TcpConnection::send(const char* str, int len) {
    /*
    append into output buffer
    enable writing
    */
    int remainBytes = len;
    int nwrite = 0;
    if (!channel_->isWriteing()) {
        assert(outputBuffer.readableBytes() == 0);
        nwrite = socket_->write(str, len);
        remainBytes = len - nwrite;
        if (remainBytes == 0 && onWriteCompleteCb)
            onWriteCompleteCb(shared_from_this());
    }
    if (remainBytes > 0) {
        outputBuffer.append(str + nwrite, remainBytes);
        if (!channel_->isWriteing()) {
            channel_->enableWriting();
            loop_->updateChannel(channel_.get());
        }
    }
}

void TcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}

void TcpConnection::shutdown() {
    // FIXME: judge if there exists data need to send before SHUT_WR
    socket_->shutdownWrite();
}
