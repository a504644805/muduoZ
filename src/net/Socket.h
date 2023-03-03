#ifndef MUDUOZ_SRC_NET_SOCKET_H
#define MUDUOZ_SRC_NET_SOCKET_H

#include <boost/noncopyable.hpp>

class SockAddrIn;
/* encapsulation of socket is benificial, after encapsulate as Socket class:
1. error handling of system calls suck as socket,bind only need to be written once.
2. RAII strategy can be adopted
*/
class Socket : boost::noncopyable {
   public:
    Socket();
    explicit Socket(int sockfd);
    ~Socket();
    void bindAddress(SockAddrIn& addr);
    void listen();
    int accept(SockAddrIn* peerAddr);
    int read(void* buf, int buflen);
    int write(const void* buf, int buflen);

    int sockfd() { return sockfd_; }

    void setTcpNoDelay(bool on);
    void shutdownWrite();

   private:
    int sockfd_;
};

namespace muduoZ {
namespace socket {
int creatNonblockingSocketOrDie();
int connect(int sockfd, const SockAddrIn& serverAddr);
int getSocketError(int sockfd);
}  // namespace socket
}  // namespace muduoZ
#endif