#ifndef MUDUOZ_SRC_NET_CALLBACK_H
#define MUDUOZ_SRC_NET_CALLBACK_H

#include <functional>
#include <memory>

class TcpConnection;
class Buffer;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
// const TcpConnectionPtr&: offer user the infomation of the TCP connection
typedef std::function<void(const TcpConnectionPtr&)> OnConnectionCb;
typedef std::function<void(const TcpConnectionPtr&)> OnCloseCb;
typedef std::function<void(const TcpConnectionPtr&, const char*, int)> OnMessageCb;
typedef std::function<void(const TcpConnectionPtr&)> OnWriteCompleteCb;

#endif