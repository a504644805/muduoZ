#ifndef MUDUOZ_SRC_NET_POLLER_H
#define MUDUOZ_SRC_NET_POLLER_H

#include <sys/epoll.h>

#include <unordered_map>
#include <vector>

#include "noncopyable.h"
class Channel;
/*
Usage of epoll:
epollfd = epoll_create
epoll_wait
epoll_ctl //used to add/mod/del epoll_event
*/
class Poller : muduoZ::noncopyable {
   public:
    typedef std::vector<Channel*> ChannelList;
    Poller();
    ~Poller();

    void poll(ChannelList& activeChannels, int timeout);

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

   private:
    typedef std::unordered_map<int, Channel*> ChannelMap;
    ChannelMap channelMap_;  // for updateChannel

    int epollfd_;
    static const int kMaxevents = 200;
    epoll_event activeEvents_[kMaxevents];  // for epoll_wait
};

#endif