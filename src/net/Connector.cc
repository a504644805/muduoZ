#include "Connector.h"

#include <assert.h>

#include "EventLoop.h"
#include "Socket.h"
Connector::Connector(const SockAddrIn& serverAddr, EventLoop* loop)
    : sockfd_(muduoZ::socket::creatNonblockingSocketOrDie()), serverAddr_(serverAddr), loop_(loop) {}
Connector::~Connector() {}

void Connector::start() {
    int rt = muduoZ::socket::connect(sockfd_, serverAddr_);
    assert(rt == -1 && errno == EINPROGRESS);  // TODO:handle different errors
    // prepare channel
}
void Connector::handleWrite() {}