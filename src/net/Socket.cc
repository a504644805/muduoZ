#include "Socket.h"

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>  // snprintf
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "Channel.h"
#include "SockAddrIn.h"
Socket::Socket() {
    sockfd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd_ < 0) {
        LOG_SYSFATAL << "socket creation failed";
    }
}

Socket::Socket(int sockfd) : sockfd_(sockfd) {
}

Socket::~Socket() {
    if (close(sockfd_) < 0) {
        LOG_SYSERR << "socket closure failed";
    }
}

void Socket::bindAddress(SockAddrIn& addr) {
    if (bind(sockfd_, reinterpret_cast<const sockaddr*>(addr.addr()), sizeof *addr.addr()) < 0) {
        LOG_SYSERR << "socket bind failed";
    }
}

void Socket::listen() {
    if (::listen(sockfd_, SOMAXCONN) < 0) {
        LOG_SYSFATAL << "socket listen failed";
    }
}

// return connfd. If peerAddr is not NULL, put into client's sockaddr_in
int Socket::accept(SockAddrIn* peerAddr) {
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);

    int connfd = ::accept4(sockfd_, reinterpret_cast<sockaddr*>(&cli), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        LOG_SYSERR << "accept failed";
    } else {
        assert(cli.sin_family == AF_INET);
        if (peerAddr) {
            peerAddr->set_sockaddr(cli);
        }
    }
    return connfd;
}

int Socket::read(void* buf, int buflen) {
    int n = static_cast<int>(::read(sockfd_, buf, buflen));  // FIXME:underly error. check all cast stuff
    if (n < 0) {
        if (errno == EAGAIN)
            ;
        else
            LOG_SYSFATAL << "read failed";
    }
    return n;
}

int Socket::write(const void* buf, int buflen) {
    int n = static_cast<int>(::write(sockfd_, buf, buflen));
    if (n < 0) {
        if (errno == EAGAIN)
            ;
        else
            LOG_SYSFATAL << "write failed";
    }
    return n;
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    if (::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval)) < 0) {
        LOG_SYSFATAL << "setsockopt failed";
    }
}

void Socket::shutdownWrite() {
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_SYSFATAL << "sockets::shutdownWrite";
    }
}

namespace muduoZ {
namespace socket {
int creatNonblockingSocketOrDie() {
    int sockfd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd_ < 0) {
        LOG_SYSFATAL << "socket creation failed";
    }
    return sockfd_;
}

int connect(int sockfd, const SockAddrIn& serverAddr) {
    return ::connect(sockfd, reinterpret_cast<const sockaddr*>(serverAddr.addr()), sizeof(struct sockaddr_in));
}

// copy from muduo
int getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        LOG_SYSFATAL << "getsockopt failed";
        return -1;  // never run to here, just make compiler happy
    } else {
        return optval;
    }
}

}  // namespace socket
}  // namespace muduoZ