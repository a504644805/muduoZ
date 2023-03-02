#include "Connector.h"

#include <assert.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
Connector::Connector(const SockAddrIn& serverAddr, EventLoop* loop)
    : sockfd_(muduoZ::socket::creatNonblockingSocketOrDie()), channel_(new Channel(sockfd_)), serverAddr_(serverAddr), loop_(loop) {}
Connector::~Connector() {}

void Connector::start() {
    int rt = muduoZ::socket::connect(sockfd_, serverAddr_);
    assert(rt == -1 && errno == EINPROGRESS);  // TODO:handle different errors
    channel_->set_onWriteableCb_(std::bind(&Connector::handleWrite, this));
    channel_->enableWriting();
    loop_->updateChannel(channel_.get());
}

void Connector::handleWrite() {
    assert(muduoZ::socket::getSocketError(sockfd_) == 0);
    loop_->removeChannel(channel_.get());
    newConnecitonCb_(sockfd_);
    // return to channel::handleEvent()
}