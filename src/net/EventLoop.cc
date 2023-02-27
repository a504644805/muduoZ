#include "EventLoop.h"

#include "Channel.h"
#include "Logger.h"
void EventLoop::loop() {
    LOG_INFO << "loop start running";
    while (1) {
        /*
        poll
        activeChannels[]->handleEvent
        */
        poller_.poll(activeChannels_, -1);
        printActiveChannels();
        for (Channel* channel : activeChannels_) {
            channel->handleEvent();
        }
    }
}

// copy from muduo
void EventLoop::printActiveChannels() const {
    for (const Channel* channel : activeChannels_) {
        LOG_TRACE << "{" << channel->reventsToString().c_str() << "} ";
    }
}
