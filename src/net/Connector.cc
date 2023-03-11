#include "Connector.h"

#include <assert.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
Connector::Connector(const SockAddrIn& serverAddr, EventLoop* loop)
    : sockfd_(muduoZ::socket::creatNonblockingSocketOrDie()), channel_(new Channel(sockfd_)), serverAddr_(serverAddr), loop_(loop) {}
Connector::~Connector() {}

void Connector::start() {
    LOG_TRACE << "::connect is called";
    int rt = muduoZ::socket::connect(sockfd_, serverAddr_);
    assert(rt == -1 && errno == EINPROGRESS);  // TODO:handle different errors
    channel_->set_onWriteableCb_(std::bind(&Connector::handleWrite, this));
    channel_->enableWriting();
    loop_->updateChannel(channel_.get());
}

void Connector::handleWrite() {
    int err = muduoZ::socket::getSocketError(sockfd_);
    if (err != 0) {
        char errnobuf[512];
        LOG_SYSFATAL << strerror_r(err, errnobuf, sizeof errnobuf);
    }
    loop_->removeChannel(channel_.get());
    newConnecitonCb_(sockfd_);
    // return to channel::handleEvent()
}