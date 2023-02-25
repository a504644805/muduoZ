#ifndef MUDUOZ_SRC_NET_POLLER_H
#define MUDUOZ_SRC_NET_POLLER_H

#include <sys/epoll.h>

#include <boost/noncopyable.hpp>

#include "Channel.h"
/*
Usage of epoll:
epollfd = epoll_create
epoll_wait
epoll_ctl //used to add/mod/del epoll_event
*/
class Poller : boost::noncopyable {
   public:
    typedef std::vector<Channel*> ChannelList;
    Poller() {
        epollfd = epoll_create1(EPOLL_CLOEXEC);
        if (epollfd < 0) {
            LOG_SYSFATAL << "epoll_create1 failed";
        }
    }
    ~Poller();

    void poll(ChannelList& activeChannels);
    void updateChannel(Channel* channel);

   private:
    int epollfd;
};

#endif