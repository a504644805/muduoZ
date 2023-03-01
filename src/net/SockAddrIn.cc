#include "SockAddrIn.h"

#include "strings.h"

SockAddrIn::SockAddrIn(int port) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(static_cast<uint16_t>(port));
}