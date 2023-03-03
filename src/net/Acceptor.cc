#include "Acceptor.h"

#include "EventLoop.h"

Acceptor::Acceptor(SockAddrIn& bindAddr, EventLoop* loop)
    : acceptSocket_(), acceptChannel_(acceptSocket_.sockfd()), loop_(loop) {  // Step1. socketfd creation
    acceptSocket_.bindAddress(bindAddr);                                      // Step2. bind
}

Acceptor::~Acceptor() {
    loop_->removeChannel(&acceptChannel_);
    // destroy acceptSocket_ (RAII object)
}

void Acceptor::start() {
    acceptSocket_.listen();          // Step3. listen
    acceptChannel_.enableReading();  // Acceptor is responsible for channel's create,destroy, register and unregister
    acceptChannel_.set_onReadableCb_(std::bind(&Acceptor::handleRead, this));
    loop_->updateChannel(&acceptChannel_);
}

void Acceptor::handleRead() {
    int connfd = acceptSocket_.accept(NULL);  // Step4. accept
    if (connfd >= 0)
        newConnectionCb(connfd);
}
