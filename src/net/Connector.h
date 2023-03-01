#ifndef MUDUOZ_SRC_NET_CONNECTOR_H
#define MUDUOZ_SRC_NET_CONNECTOR_H

#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>

#include "SockAddrIn.h"

// TODO:support retry
class Channel;
class SockAddrIn;
class EventLoop;
class Connector : public boost::noncopyable {
   public:
    typedef std::function<void(int)> NewConnecitonCb;
    Connector(const SockAddrIn& serverAddr, EventLoop* loop);
    ~Connector();

    void start();
    void handleWrite();
    void set_newConnecitonCb(const NewConnecitonCb& cb) { newConnecitonCb_ = cb; }

   private:
    // temporary
    int sockfd_;  // not class Socket because we want offer TcpConnection a "raw connection", hence we don't want sockfd be closed by class Socket's destructor
    std::unique_ptr<Channel> channel_;

    NewConnecitonCb newConnecitonCb_;
    SockAddrIn serverAddr_;
    EventLoop* loop_;
};
#endif