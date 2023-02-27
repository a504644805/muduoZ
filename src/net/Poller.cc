#include "Poller.h"

#include "Channel.h"

Poller::Poller() {
    if ((epollfd_ = epoll_create1(EPOLL_CLOEXEC)) < 0)
        LOG_SYSFATAL << "epoll_create1 failed";
}
Poller::~Poller() {
    if (close(epollfd_) < 0)
        LOG_SYSERR << "close epollfd_ failed";
}

// TODO:Use vector for activeEvents_ to dynamically expand when there are more events (good for perfromance?)
void Poller::poll(ChannelList& activeChannels, int timeout) {
    int num = 0;
    if ((num = epoll_wait(epollfd_, activeEvents_, kMaxevents, timeout)) < 0)
        LOG_SYSFATAL << "epoll_wait failed";

    activeChannels.clear();
    for (int i = 0; i < num; i++) {
        Channel* channel = static_cast<Channel*>(activeEvents_[i].data.ptr);
        assert(channelMap_.find(channel->fd()) != channelMap_.end());
        assert(channelMap_.find(channel->fd())->second == channel);
        channel->set_revents(activeEvents_[i].events);
        activeChannels.push_back(channel);
    }
}

void Poller::updateChannel(Channel* channel) {
    epoll_event epollEvent;
    epollEvent.events = channel->events();
    epollEvent.data.ptr = channel;  //!!!
    int op;
    /*
    if it's a new channel for epoll_fd, add
    if the channel already in epoll_fd, mod
    */
    ChannelMap::iterator it = channelMap_.find(channel->fd());

    if (it == channelMap_.end()) {  // add
        channelMap_[channel->fd()] = channel;
        op = EPOLL_CTL_ADD;
    } else {  // mod
        channelMap_[channel->fd()] = channel;
        op = EPOLL_CTL_MOD;
    }
    if (epoll_ctl(epollfd_, op, channel->fd(), &epollEvent) < 0) {
        LOG_SYSFATAL << "epoll_ctl failed";
    }
}

void Poller::removeChannel(Channel* channel) {
    ChannelMap::iterator it = channelMap_.find(channel->fd());
    if (it != channelMap_.end()) {
        channelMap_.erase(it);
        if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, channel->fd(), NULL) < 0) {
            LOG_SYSFATAL << "epoll_ctl failed";
        }
    }
}