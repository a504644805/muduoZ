#include <iostream>

#include "AsyncLogging.h"
#include "EventLoop.h"
using namespace std;
using namespace muduoZ;
int main() {
    AsyncLogging logger;
    logger.start();
    logger.append("testlog1", 8);

    EventLoop loop;
    loop.loop();

    return 0;
}
