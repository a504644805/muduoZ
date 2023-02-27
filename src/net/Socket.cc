#include "Socket.h"

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "Channel.h"
#include "SockAddrIn.h"
Socket::Socket() {
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ < 0) {
        LOG_SYSFATAL << "socket creation failed";
    }
}

Socket::~Socket() {
    if (close(sockfd_) < 0) {
        LOG_SYSERR << "socket closure failed";
    }
}

void Socket::bindAddress(SockAddrIn& addr) {
    if (bind(sockfd_, (sockaddr*)addr.addr(), sizeof *addr.addr()) < 0) {
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

    int connfd = ::accept4(sockfd_, (sockaddr*)&cli, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
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