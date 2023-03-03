#include <stdio.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <utility>

#include "Atomic.h"
#include "Buffer.h"
#include "EventLoop.h"
#include "Logger.h"
#include "SockAddrIn.h"
#include "TcpClient.h"
#include "TcpConnection.h"

using namespace std;
using namespace std::placeholders;
using namespace muduoZ;

class Client;
class Session {
   public:
    Session(EventLoop* loop,
            const SockAddrIn& serverAddr,
            const string& name,
            Client* owner)
        : client_(serverAddr, loop),
          owner_(owner),
          bytesRead_(0),
          bytesWritten_(0),
          messagesRead_(0) {
        client_.set_onConnectionCb(
            std::bind(&Session::onConnection, this, _1));
        client_.set_onCloseCb(
            std::bind(&Session::onClose, this, _1));
        client_.set_onMessageCb(
            std::bind(&Session::onMessage, this, _1, _2));
        client_.set_onWriteCompleteCb(
            std::bind(&Session::onWriteComplete, this, _1));
    }

    void start() {
        client_.start();
    }

    void stop() {
        client_.disconnect();
    }

    int64_t bytesRead() const {
        return bytesRead_;
    }

    int64_t messagesRead() const {
        return messagesRead_;
    }

   private:
    void onConnection(const TcpConnectionPtr& conn);
    void onClose(const TcpConnectionPtr& conn);

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf) {
        ++messagesRead_;
        bytesRead_ += buf->readableBytes();
        bytesWritten_ += buf->readableBytes();
        conn->send(buf->readPtr(), buf->readableBytes());
        buf->retrieveAll();
    }
    void onWriteComplete(const TcpConnectionPtr& conn) {}

    TcpClient client_;
    Client* owner_;
    int64_t bytesRead_;
    int64_t bytesWritten_;
    int64_t messagesRead_;
};

class Client {
   public:
    Client(EventLoop* loop,
           const SockAddrIn& serverAddr,
           int blockSize,
           int sessionCount,
           int timeout,
           int threadCount)
        : loop_(loop),
          sessionCount_(sessionCount),
          timeout_(timeout) {
        loop->runAfter(timeout, std::bind(&Client::handleTimeout, this));

        for (int i = 0; i < blockSize; ++i) {
            message_.push_back(static_cast<char>(i % 128));
        }

        for (int i = 0; i < sessionCount; ++i) {
            char buf[32];
            snprintf(buf, sizeof buf, "C%05d", i);
            Session* session = new Session(loop, serverAddr, buf, this);
            session->start();
            sessions_.emplace_back(session);
        }
    }

    const string& message() const {
        return message_;
    }

    void onConnect() {
        if (numConnected_.incrementAndGet() == sessionCount_) {
            LOG_WARN << "all connected";
        }
    }

    void onDisconnect(const TcpConnectionPtr& conn) {
        LOG_SYSFATAL << "This won't happen";
    }

   private:
    void handleTimeout() {
        int64_t totalBytesRead = 0;
        int64_t totalMessagesRead = 0;
        for (const auto& session : sessions_) {
            totalBytesRead += session->bytesRead();
            totalMessagesRead += session->messagesRead();
        }
        cout << totalBytesRead << " total bytes read" << endl;
        cout << totalMessagesRead << " total messages read" << endl;
        cout << static_cast<double>(totalBytesRead) / static_cast<double>(totalMessagesRead)
             << " average message size" << endl;
        cout << static_cast<double>(totalBytesRead) / (timeout_ * 1024 * 1024)
             << " MiB/s throughput" << endl;
        loop_->queueInLoop(std::bind(&EventLoop::quit, loop_));
    }

    EventLoop* loop_;
    // EventLoopThreadPool threadPool_;
    int sessionCount_;
    int timeout_;
    std::vector<std::unique_ptr<Session>> sessions_;
    string message_;
    AtomicInt32 numConnected_;
};

void Session::onConnection(const TcpConnectionPtr& conn) {
    conn->setTcpNoDelay(true);
    conn->send(owner_->message().c_str(), static_cast<int>(owner_->message().size()));
    owner_->onConnect();
}

void Session::onClose(const TcpConnectionPtr& conn) {
    owner_->onDisconnect(conn);
}

extern AsyncLogging g_asynclogging;
int main(int argc, char* argv[]) {
    if (argc < 7) {  // FIXME: should be !=7. but to debug in vscode we use < 7
        fprintf(stderr, "Usage: client <host_ip> <port> <threads> <blocksize> ");
        fprintf(stderr, "<sessions> <time>\n");
    } else {
        g_asynclogging.start();

        LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::getTid();

        // const char* ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        int threadCount = atoi(argv[3]);
        int blockSize = atoi(argv[4]);
        int sessionCount = atoi(argv[5]);
        int timeout = atoi(argv[6]);

        EventLoop loop;
        SockAddrIn serverAddr(port);

        Client client(&loop, serverAddr, blockSize, sessionCount, timeout, threadCount);
        loop.loop();
        g_asynclogging.stop();
    }
    return 0;
}
