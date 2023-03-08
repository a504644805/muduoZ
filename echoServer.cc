#include <functional>
#include <iostream>

#include "Buffer.h"
#include "EventLoop.h"
#include "Logger.h"
#include "SockAddrIn.h"
#include "TcpConnection.h"
#include "TcpServer.h"
using namespace std;
using namespace std::placeholders;
using namespace muduoZ;

class EchoServer {
   public:
    EchoServer(EventLoop* loop, SockAddrIn& listenAddr)
        : loop_(loop), server_(listenAddr, loop) {
        server_.set_onConnectionCb(
            std::bind(&EchoServer::onConnection, this, _1));
        server_.set_onCloseCb(
            std::bind(&EchoServer::onClose, this, _1));
        server_.set_onMessageCb(
            std::bind(&EchoServer::onMessage, this, _1, _2));
        server_.set_onWriteCompleteCb(
            std::bind(&EchoServer::onWriteComplete, this, _1));
    }

    void start() { server_.start(); }

   private:
    void onConnection(const TcpConnectionPtr& conn);
    void onClose(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf);
    void onWriteComplete(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    TcpServer server_;
};

void EchoServer::onConnection(const TcpConnectionPtr& conn) {
    LOG_TRACE << "connection established";
}
void EchoServer::onClose(const TcpConnectionPtr& conn) {
    LOG_TRACE << "connection disconnected";
}
void EchoServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
    // string msg(buf->retrieveAllAsString());
    // LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " <<
    // time.toString(); conn->send(msg);
    conn->send(buf->readPtr(), buf->readableBytes());
    buf->retrieveAll();
}
void EchoServer::onWriteComplete(const TcpConnectionPtr& conn) {
}

extern AsyncLogging g_asynclogging;
int main() {
    g_asynclogging.start();

    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::getTid();

    EventLoop loop;
    SockAddrIn listenAddr(2007);
    EchoServer server(&loop, listenAddr);

    server.start();

    loop.loop();

    /*
        AsyncLogging logger;
        logger.start();
        logger.append("testlog1", 8);
    */
    return 0;
}
