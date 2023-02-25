#include <iostream>

#include "EventLoop.h"
#include "Logger.h"
using namespace std;
using namespace muduoZ;
extern AsyncLogging g_asynclogging;
int main() {
    LOG_INFO << "Program Started, " << 123321;
    LOG_INFO << "Program Started, " << 321123;

    g_asynclogging.start();

    EventLoop loop;
    loop.loop();

    /*
        AsyncLogging logger;
        logger.start();
        logger.append("testlog1", 8);
    */
    return 0;
}
