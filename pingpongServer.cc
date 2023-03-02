#include <stdio.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <utility>

#include "Buffer.h"
#include "EventLoop.h"
#include "Logger.h"
#include "SockAddrIn.h"
#include "TcpConnection.h"
#include "TcpServer.h"
using namespace std;
using namespace std::placeholders;
using namespace muduoZ;

void onConnection(const TcpConnectionPtr& conn) {
    conn->setTcpNoDelay(true);
}

void onClose(const TcpConnectionPtr& conn) {
    conn->setTcpNoDelay(true);
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
    conn->send(buf->readPtr(), buf->readableBytes());
    buf->retrieveAll();
}

void onWriteComplete(const TcpConnectionPtr& conn) {
}

int main(int argc, char* argv[]) {
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::getTid();

    SockAddrIn listenAddr(2008);

    // int threadCount = 1;

    EventLoop loop;

    TcpServer server(listenAddr, &loop);
    server.set_onConnectionCb(std::bind(onConnection, _1));
    server.set_onCloseCb(std::bind(onClose, _1));
    server.set_onMessageCb(std::bind(onMessage, _1, _2));
    server.set_onWriteCompleteCb(std::bind(onWriteComplete, _1));

    // if (threadCount > 1) {
    //     server.setThreadNum(threadCount);
    // }

    server.start();

    loop.loop();
}
