#ifndef MUDUOZ_SRC_NET_SOCKADDRIN_H
#define MUDUOZ_SRC_NET_SOCKADDRIN_H

#include <netinet/in.h>

// benificial after encapsulation:
// 1. some dirty work such as bzero needn't to be done by user
class SockAddrIn {
   public:
    explicit SockAddrIn(int port);
    ~SockAddrIn(){};
    const struct sockaddr_in* addr() const { return &addr_; }
    void set_sockaddr(const struct sockaddr_in& addr) { addr_ = addr; }

   private:
    struct sockaddr_in addr_;
};

#endif