#ifndef MUDUOZ_SRC_NET_POLLER_H
#define MUDUOZ_SRC_NET_POLLER_H

#include <sys/epoll.h>

#include <boost/noncopyable.hpp>
#include <unordered_map>

class Channel;
/*
Usage of epoll:
epollfd = epoll_create
epoll_wait
epoll_ctl //used to add/mod/del epoll_event
*/
class Poller : boost::noncopyable {
   public:
    typedef std::vector<Channel*> ChannelList;
    Poller();
    ~Poller();

    void poll(ChannelList& activeChannels, int timeout);

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

   private:
    int epollfd_;

    typedef std::unordered_map<int, Channel*> ChannelMap;
    ChannelMap channelMap_;  // for updateChannel

    static const int kMaxevents = 25;
    epoll_event activeEvents_[kMaxevents];  // for epoll_wait
};

#endif