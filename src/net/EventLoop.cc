#include "EventLoop.h"
void EventLoop::loop() {
    printf("loop start running\n");
    sleep(3);
    printf("loop stop running\n");
}