#include <sys/socket.h>

#include <functional>
#include <iostream>

#include "Buffer.h"
#include "EventLoop.h"
#include "Logger.h"
#include "SockAddrIn.h"
#include "TcpClient.h"
#include "TcpConnection.h"
#include "TcpServer.h"
#include "assert.h"
using namespace std;
using namespace std::placeholders;
using namespace muduoZ;

void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << "Connection established";
    int msgSz = 8192 * 1000;
    std::string msg(msgSz, 'm');
    // LOG_INFO << "shutdown read";
    // conn->shutdownRead();
    LOG_INFO << "Send messag";
    conn->send(msg.c_str(), msg.size());  // now into socket buffer
    conn->forceClose();
}
void onClose(const TcpConnectionPtr& conn) {
    LOG_INFO << "Connection disconnected";
}
void onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
    // string msg(buf->retrieveAllAsString());
    // LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " <<
    // time.toString(); conn->send(msg);

    // conn->send(buf->readPtr(), buf->readableBytes());
    buf->retrieveAll();
}
void onWriteComplete(const TcpConnectionPtr& conn) {
    LOG_INFO << "All into buffer";
}

extern AsyncLogging g_asynclogging;
int mainpid = 0;
int main() {
    g_asynclogging.start();

    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::getTid();
    mainpid = getpid();
    EventLoop loop;
    SockAddrIn serverAddr(2007);
    TcpClient client(serverAddr, &loop);
    client.set_onConnectionCb(onConnection);
    client.set_onCloseCb(onClose);
    client.set_onMessageCb(onMessage);
    client.set_onWriteCompleteCb(onWriteComplete);
    client.start();

    loop.loop();
    return 0;
}
