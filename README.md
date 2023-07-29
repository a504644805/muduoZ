muduoZ
---

<a href="https://github.com/Qihoo360/evpp/releases"><img src="https://img.shields.io/github/release/Qihoo360/evpp.svg" alt="Github release"></a>
<a href="https://travis-ci.org/Qihoo360/evpp"><img src="https://travis-ci.org/Qihoo360/evpp.svg?branch=master" alt="Build status"></a>
[![Platform](https://img.shields.io/badge/platform-%20%20%20%20Linux-green.svg?style=flat)](https://github.com/Qihoo360/evpp)
[![License](https://img.shields.io/badge/license-%20%20BSD%203%20clause-yellow.svg?style=flat)](LICENSE)
[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)

## 0. 写在前面

这是一个基于C++11的muduo网络库。

## 1. 项目概述

在阅读完[muduo](https://github.com/chenshuo/muduo)的源码，并通过单步调试对其有一定理解后，出于学习和更深入地理解muduo，笔者重写了muduo网络库。muduoZ相比muduo主要有如下特点：

1. 使用C++11/14/17的一些特性替换muduo原有操作，如使用C++11引入的std::mutex代替muduo的mutex进行RAII的互斥锁操作。
2. 移除了Muduo库中的Boost依赖。如使用自C++17引入的std::any替代boost::any；移除boost::less_than_comparable，并手动添加了缺失的运算符重载。（参考了boost::less_than_comparable的实现，使新添加的运算符重载是通过<运算符实现的）
3. 在异步日志中，muduo用的是[fwrite+flush](https://github.com/chenshuo/muduo/blob/f29ca0ebc2f3b0ab61c1be08482a5524334c3d6f/muduo/base/AsyncLogging.cc#L99)完成page-cache的写入，muduoZ用的则是[writev](https://github.com/a504644805/muduoZ/blob/b97289d38e693204a34fc1ec7fc1c31bfd5a3064/src/base/AsyncLogging.cc#L57)。相比前者，writev的系统调用次数可以少一些，从而节省操作系统在用户态和内核态之间的切换耗时。
4. 在Thread类的实现中，为获得子线程的唯一标识（tid），muduoZ采用[lambda](https://github.com/a504644805/muduoZ/blob/b97289d38e693204a34fc1ec7fc1c31bfd5a3064/src/base/Thread.cc#L6)表达式作为线程函数的实参。相比muduo传入一个[间接](https://github.com/chenshuo/muduo/blob/f29ca0ebc2f3b0ab61c1be08482a5524334c3d6f/muduo/base/Thread.cc#L179)的函数，代码更简洁易懂。同时muduoZ也使用了C++11引入的std::thread进行线程创建等相关操作。
5. 不同于muduo直接把cookie放进FixedBuffer类，muduoZ对FixedBuffer进行了一次派生，让日志系统相关的类使用FixedBufferWithCookie。这样其它非日志系统相关的类如果想使用FixedBuffer，就不会因为也有着一样的cookie，从而和日志消息发生混淆。
6. 相比muduo，muduoZ只重写了核心部分。没有实现诸如TimeZone的辅助类。

### 1.1 总览

muduo是由[chen shuo](https://github.com/chenshuo)所写基于Reactor架构的高性能C++网络库。

通过阅读源码，我将构成muduo的框架划分为三块内容：

1）由EventLoop、Poller、Channel构成的Reactor框架

2）由TcpServer、Acceptor、TcpConnection构成的Server三件套

3）由TcpClient、Connector、(TcpConnection)构成的Client三件套

![UML_Class_Graph](https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/UML_Class_Graph.png)

整体的UML类图如上所示（为突出重点，只画出核心类及各类的核心成员）。其中由EventLoop、Poller、Channel构成的Reactor框架是muduo的核心，EventLoop的loop函数会循环往复地进行

1.调用poll/epoll （由Poller封装)。

2.处理I/O事件

Server/Client则通过Acceptor/Connector来进行连接的接收/发起。其中Server可以管理多条连接（TcpConnection），Client则最多只对应一条连接。TcpConnection、Acceptor、Connector都会通过Channel来接入Reactor框架（2.1 对如何接入进行了介绍）。

## 2. 几点心得

关于muduo，[《Linux多线程服务端编程》](https://book.douban.com/subject/20471211//)以及众多博客都有很多介绍，在此就不重复前人发言了。挑几点我认为比较重要但很少见人说起的理解。

### 2.1 Channel==黏合剂

要深刻理解总览中提到的三部分间的关联，关键是把握Channel类作为**黏合剂**的本质。

只看Channel，会发现其负责某个socket fd的事件分发（handleEvent函数），却又不负责socket fd的生命周期，好像是个非常普通又有些奇怪的类。

但当我们从整体来观察，就会发现Channel和Poller属于聚合关系；和Acceptor，Connector，TcpConnection属于组合关系；EventLoop也用到了Channel类来进行唤醒操作，简直无所不在。这本质上是因为muduo采用Channel到Poller的注册和注销机制实现I/O事件的动态增删。

实现机制如下：

1. Channel及“Channel负责的socket fd”的生命周期和Channel的拥有者一致（拥有者包括 Acceptor，Connector，TcpConnection etc. )

2. Channel的拥有者负责Channel的创建、配置、注册、注销这一系列动作。

以TcpConnection为例进行说明（下面的代码来自muduoZ，muduo的做法也类似）：

1. 以unique_ptr来管理Channel，两者的生命期一致

   ```c++
   private:
   	std::unique_ptr<Channel> channel_;
   ```

2. 负责Channel的创建、配置、注册和注销

   - TcpConnection的构造函数负责Channel的创建

     ```c++
     TcpConnection::TcpConnection(int sockfd, EventLoop* loop) : channel_(new Channel(sockfd))
     ```

   * 当新连接到达，Acceptpr会接受连接并回调TcpServer::newConnection，后者会调用TcpConnection::connEstablished，该函数负责Channel的配置，并将其注册到Poller中去。

     ```c++
     void TcpConnection::connEstablished() {
         channel_->set_onReadableCb_(std::bind(&TcpConnection::handleRead, this));
         channel_->set_onWriteableCb_(std::bind(&TcpConnection::handleWrite, this));
         channel_->set_onCloseCb_(std::bind(&TcpConnection::handleClose, this));
         channel_->enableReading();
         channel_->tie(shared_from_this());
         loop_->updateChannel(channel_.get());
     	//...
     }
     ```

   * 当连接断开，TcpConnection::handleClose会把Channel从Poller中移出

     ```c++
     void TcpConnection::handleClose() {
         loop_->removeChannel(channel_.get());
     	//...
     }
     ```



### 2.2 如何优雅地结束

陈硕在《Linux多线程服务端编程》中曾写道：

> ”一般来讲数据的删除比新建要复杂，TCP 连接也不例外。关闭连接的流程看上去有点“绕”，根本原因是对象生命期管理的需要“

在由Qihoo360所开发的[evpp](https://github.com/Qihoo360/evpp)网络库中也有提及：

> 我们如此苛刻的追求线程安全，只是为了让一个程序能安静的平稳的退出或Reload。因为我们深刻的理解“编写永远运行的系统，和编写运行一段时间后平静关闭的系统是两码事”，后者要困难的多得多。

如他们所言，优雅的结束往往比endless的loop更难，需要考虑更多的东西。

在muduoZ中，主要涉及的资源释放包括：线程的销毁、内存空间的释放、连接的断开。通过在初始化阶段完成全部子线程的创建，避免了在程序运行期间需要动态创建和销毁线程的烦恼。而内存空间等资源的释放，则借助智能指针，并配合RAII的思想，使对象在析构时自动释放资源。

在这我主要想介绍muduoZ关于连接断开的处理。尝试打通TCP协议、TCP的具体实现（Linux内核中的TCP/IP协议栈）以及应用开发者应该处理的事件（采用epoll观察和处理连接断开时相关的I/O事件）三者间的联系。下述源码的内核版本皆为 **Linux 5.4.0**。

```C++
                	          +---------+                              
                              |  ESTAB  |                              
                              +---------+                              
                       CLOSE    |     |    rcv FIN                     
                      -------   |     |    -------                     
 +---------+          snd FIN  /       \   snd ACK          +---------+
 |  FIN    |<-----------------           ------------------>|  CLOSE  |
 | WAIT-1  |------------------                              |   WAIT  |
 +---------+          rcv FIN  \                            +---------+
   | rcv ACK of FIN   -------   |                            CLOSE  |  
   | --------------   snd ACK   |                           ------- |  
   V        x                   V                           snd FIN V  
 +---------+                  +---------+                   +---------+
 |FINWAIT-2|                  | CLOSING |                   | LAST-ACK|
 +---------+                  +---------+                   +---------+
   |                rcv ACK of FIN |                 rcv ACK of FIN |  
   |  rcv FIN       -------------- |    Timeout=2MSL -------------- |  
   |  -------              x       V    ------------        x       V  
    \ snd ACK                 +---------+delete TCB         +---------+
     ------------------------>|TIME WAIT|------------------>| CLOSED  |
                              +---------+                   +---------+
```

上图摘自[RFC793](https://www.ietf.org/rfc/rfc793.txt)的TCP状态转换图，我们只关注涉及连接断开涉及的部分。可以发现其涉及的 \"连接断开起始条件\" 只包括发送FIN或接收FIN两种情况，然后在实际场景中，当我们的服务器和客户端建立连接后，客户端可能执行的涉及连接断开的操作包括：

* 调用close断开连接
* 调用shutdown关闭 读和 (或) 写
* 电源掉电，客户端什么都没来得及发送（通过设置TCP的KeepAlive标志位，服务端即可侦测到异常并断开连接）

下面介绍前两种情况。

第一种情况下，

**当客户端**调用close，如果此时socket的receive buffer还有未读数据，则会将状态直接置为CLOSE并发送RST报文关闭连接；如果数据已经读完同时未开启SOCK_LINGER标志，则会调用`tcp_send_fin`发送FIN报文，此时状态变为FIN_WAIT-1。这里介绍前一种情况，后一种情况在 \"调用shutdown关闭 读和 (或) 写\" 中会介绍。

```c
void tcp_close(struct sock *sk, long timeout){
    ......
	/* As outlined in RFC 2525, section 2.17, we send a RST here because
	 * data was lost. To witness the awful effects of the old behavior of
	 * always doing a FIN, run an older 2.1.x kernel or 2.0.x, start a bulk
	 * GET in an FTP client, suspend the process, wait for the client to
	 * advertise a zero window, then kill -9 the FTP client, wheee...
	 * Note: timeout is always zero in such a case.
	 */
	if (unlikely(tcp_sk(sk)->repair)) {
		sk->sk_prot->disconnect(sk, 0);
	} else if (data_was_unread) {
		/* Unread data was tossed, zap the connection. */
		NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPABORTONCLOSE);
		tcp_set_state(sk, TCP_CLOSE);
		tcp_send_active_reset(sk, sk->sk_allocation);
	} else if (sock_flag(sk, SOCK_LINGER) && !sk->sk_lingertime) {
		/* Check zero linger _after_ checking for unread data. */
		sk->sk_prot->disconnect(sk, 0);
		NET_INC_STATS(sock_net(sk), LINUX_MIB_TCPABORTONDATA);
	} else if (tcp_close_state(sk)) {
		tcp_send_fin(sk);
	}
	......
}
```

**当服务端**收到来自客户端的RST报文，内核会依次调用如下函数：

```
tcp_v4_rcv
	-> tcp_v4_do_rcv
		-> tcp_rcv_established
			-> tcp_validate_incoming
				-> tcp_reset
```

在`tcp_reset`中，如果此时TCP连接处于ESTABLISH状态，表示连接首次收到RST，此时会将错误置为ECONNRESET。此时如果用户对该socket fd调用write函数，errno会置为104（ECONNRESET）。随后`tcp_done`被调用，连接被置为CLOSE状态。

```C
/* When we get a reset we do this. */
void tcp_reset(struct sock *sk)
{
	trace_tcp_receive_reset(sk);

	/* We want the right error as BSD sees it (and indeed as we do). */
	switch (sk->sk_state) {
	case TCP_SYN_SENT:
		sk->sk_err = ECONNREFUSED;
		break;
	case TCP_CLOSE_WAIT:
		sk->sk_err = EPIPE;
		break;
	case TCP_CLOSE:
		return;
	default:
		sk->sk_err = ECONNRESET;
	}
	......
	tcp_done(sk);
    ......
}
```

**对应地，在服务端，muduoZ**所调用的epoll会返回EPOLLHUP事件，于是调用TcpConnection::handleClose函数来进行资源释放的相关操作。



第二种情况下，

当客户端调用`shutdown (fd, SHUT_RD)`，此时客户端不会向服务端发送FIN信息，只是当用户调用read会报错。

**当客户端**调用**`shutdown (fd, SHUT_WR)`**，协议栈的`tcp_shutdown`函数会通过`tcp_close_state`函数将状态置为FIN_WAIT-1，并发送FIN报文。

```C
void tcp_shutdown(struct sock *sk, int how)
{
	......
	if ((1 << sk->sk_state) &
	    (TCPF_ESTABLISHED | TCPF_SYN_SENT |
	     TCPF_SYN_RECV | TCPF_CLOSE_WAIT)) {
		/* Clear out any half completed packets.  FIN if needed. */
		if (tcp_close_state(sk))
			tcp_send_fin(sk);
	}
}
```

**当服务端**收到来自客户端的RST报文，内核会依次调用如下函数：

```C
tcp_v4_rcv
	-> tcp_v4_do_rcv
		-> tcp_rcv_established
			-> tcp_data_queue
				-> tcp_fin
			-> tcp_ack_snd_check
```

在`tcp_fin`中，会将TCP连接置于CLOSE_WAIT状态，等待应用层关闭tcp连接。

可以看到在`tcp_fin`还进行了`sk->sk_shutdown |= RCV_SHUTDOWN`，表示读端关闭。对应地，客户端在调用`shutdown (fd, SHUT_WR)`时会进行`sk->sk_shutdown |= SEND_SHUTDOWN`，表示写端关闭。

```C
void tcp_fin(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);

	inet_csk_schedule_ack(sk);

	sk->sk_shutdown |= RCV_SHUTDOWN;
	sock_set_flag(sk, SOCK_DONE);

	switch (sk->sk_state) {
	case TCP_SYN_RECV:
	case TCP_ESTABLISHED:
		/* Move to CLOSE_WAIT */
		tcp_set_state(sk, TCP_CLOSE_WAIT);
		inet_csk_enter_pingpong_mode(sk);
		break;
```

随后`tcp_rcv_established`会调用`tcp_ack_snd_check`发送ack。

**当客户端**收到来自服务端的ack回复，会由FIN_WAIT-1状态变为FIN_WAIT-2，调用链如下：

```C
tcp_v4_rcv
	-> tcp_v4_do_rcv
		-> tcp_rcv_state_process
```

**现在切换到服务端的muduoZ视角**。此时由客服端调用`shutdown (fd, SHUT_WR)`而引发的一系列连锁效应已经由TCP/IP协议栈完成。客户端处于FIN_WAIT-2状态，等着FIN报文的到来。服务端处于CLOSE_WAIT状态，等着应用层关闭tcp连接。

此时muduoZ有两种方式得知此状态

1. 通过epoll返回的EPOLLRDHUP

2. epoll会返回EPOLLIN，此时调用read会返回0

一旦感知到该状态，muduoZ就会调用TcpConnection::handleClose函数进入资源释放的流程。当TcpConnection析构时，其拥有的由RAII机制管理的Socket对象会调用**close**函数关闭连接。

随后会进行（涉及到的核心函数在上面已有罗列，这儿就不赘述代码了）：

* 服务端会发送FIN报文并进入LAST_ACK状态。
* 客户端收到FIN报文后先回复ack，随后进入TIME_WAIT状态并开启TIME_WAIT定时。
* 服务端收到ack后进入CLOSE状态，客户端定时器超时后也进入CLOSE状态。

以上便是从协议、Linux TCP协议栈的实现、开发人员三个角度，自底向上地针对TCP \"连接断开\" 所展开的叙述。怎样辨识连接断开的发生，连接断开时又应该做什么。这个在编写muduoZ时始终困扰我的问题终于有了一些解答。也希望自己能早日达到硕哥在书中提到的网络编程的三个层次：

> * 读过教程和文档，做过练习
> * 熟悉本系统 TCP/IP 协议栈的脾气
> * 自己写过一个简单的 TCP/IP stack

### 2.3 一条日志消息都别想跑

作为分布式系统中事故调查的重要线索，我们自然不希望遗漏任何日志消息，特别是程序崩溃前 “遗留在内存中尚未写入磁盘” 的日志消息。通过在 “缓存日志消息的类” 中添加cookie数据成员，并在构造和析构时将其置为特定的魔数，我们便可以通过core文件快速定位尚未写入磁盘的日志消息。

muduoZ中，FixedBufferWithCookie作为缓存日志消息的类，在构造函数时会将cookie数据成员置为其静态函数成员cookieStart的地址，析构时则会将其置为cookieEnd。

```C++
template <int SIZE>
class FixedBuffer {
   public:
	//...
    void setCookie(void (*cookie)()) { cookie_ = cookie; }
    
   private:
    void (*cookie_)();
    char data[SIZE];
    char* cur;
};

template <int SIZE>
class FixedBufferWithCookie: public FixedBuffer<SIZE> { // Just for Logging
   public:
    FixedBufferWithCookie() { this->setCookie(cookieStart); }
    ~FixedBufferWithCookie() { this->setCookie(cookieEnd); }

   private:
    static void cookieStart(){};
    static void cookieEnd(){};
};
```

为了区分 “已经写入page-cache的日志消息” 和 “尚未写入page-cache，仍存在于内存中的日志消息” ，muduoZ会在每次调用writev（此时消息已写入page cache）后才析构FixedBufferWithCookie。这样，在查看core文件时，我们只需要查找cookieStart即可。

```C++
void AsyncLogging::threadFunc() {
	//...
	const ssize_t n = writev(fd, vec, iovcnt);
	if (n < 0) {
    	//...
	}
    //...
	buffersToWrite.clear();
```

由于在网上没有找到 “如何由cookie查找日志消息” 的具体操作，因此在这演示下笔者所采用的方法：

1. 在echoServer.cc中故意进行除0操作让其core dump得到core文件。此时 `LOG_INFO << "pid = " << ...` 对应的消息还没来得及写入磁盘。

   ```C++
   int main() {
   	g_asynclogging.start();
   	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::getTid();
   	
   	int a=0;
   	int b=2/a;
   	b++;
   }
   ```

2. 为了找到遗失的消息，先通过gdb查得cookieStart函数的地址

   ![2023-03-07_141503](https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/gdb.png)

3. 随后借助xxd查看core文件。由于xxd显示的是小端字节序，因此我们查找的内容是0x0000562c4859138f的小端字节序，即0x8f1359482c560000。可以看到紧随其后的便是日志消息。

   ![2023-03-07_141323](https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/xxd.png)

 

### 2.4 别被回调绕晕了

无意中看见知乎一个调侃muduo回调逻辑复杂的评论，仔细想想初看muduo确实花了一点时间捋清楚里面的回调逻辑，因此一并写了罢。

要厘清回调，首先要提炼出为了方便理解和编程的一些约定：

1. 原封不动地传递：TcpServer和TcpClient会把从用户那拿到的回调函数并原封不动传给TcpConnection。最终由TcpConnection进行用户函数的回调。
2. “事件分发” 和 “具体行为逻辑” 两个任务的切割：Channel的handleEvent专职负责事件的分发；TcpConnection的handleRead，handleWrite等函数专职负责具体行为逻辑。配置Channel时，TcpConnection会将自己的handleRead，handleWrite等函数传给Channel。当I/O事件如EPOLLIN发生时，Channel的handleEvent会回调TcpConnection的handleRead，后者会读取socket缓冲区中的数据到用户缓冲。

基于上述自上往下（用户到Channel）的约定，再结合代码画出自下往上（Channel到用户）的连接建立、通信、断开的时序图，便可理解muduo的回调逻辑。如下为通信过程中，当接收到EPOLLIN和EPOLLOUT时对应的时序图。回调函数已经用加粗标示出来了。

<img src="https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/seq.png" style="zoom: 25%;" />

<img src="https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/seq2.png" style="zoom: 34%;" />

连接的建立比较复杂，当Acceptor的acceptChannel收到EPOLLIN后，会回调Acceptor的handleRead函数，该函数在accept连接后再回调TcpServer的newConnection函数，提供给其一条“raw connection”（为便于理解的一个名词，指新连接的socket fd，和TcpConnection类相对应）。newConnection函数会新建TcpConnection并调用其connEstablished函数完成channel的配置、注册操作。

<img src="https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/seq3.png" style="zoom: 33%;" />

连接断开的可能性有多种，下面的图是当发生POLLIN但TcpConnection读到0字节情况下发生的连接断开。

<img src="https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/seq4.png" style="zoom: 46%;" />

### 2.5 为何要tie

在重写muduo时，我有一条原则：遇见自己难以理解的手法，在查阅资料无果后，先标注，然后按照自己的理解去写而不是原样照搬。这虽然让我吃了不少苦头（花在调试程序的时间更久了；刚写完发现muduoZ性能比muduo差了三倍，花了几天才找到所有病症)，但收获也不少，理解为何Channel::handleEvent在进行事件分发之前要进行如下操作便是收获之一。

```C++
void Channel::handleEvent() {
    std::shared_ptr<void> guard;
    if (tied_) {
        guard = tie_.lock();
        assert(guard);
    }
    // ...
}
```

在Channel类中有两个相关的数据成员：

```c++
class Channel{
    // ...
private:
    std::weak_ptr<void> tie_;
    bool tied_;
};
```

TcpServer/TcpClient通过std::shared_ptr管理TcpConnection：

```C++
class TcpServer{
    // ...
private:
	typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
	std::unordered_set<TcpConnectionPtr> connectionSet_;
};
```

当由TcpServer/TcpClient管理的TcpConnection在连接建立 配置自己的Channel时，会调用Channel的tie函数，让Channel的tie\_指向自己：

```c++
void TcpConnection::connEstablished() {  // called by TcpServer::newConnection
	// ...
    channel_->tie(shared_from_this());
	// ...
}
```

于是，当Channel将tie\_进行提升后，就可以保证hanleEvent期间TcpConnection不会因为std::shared\_ptr的引用计数降为0而导致TcpConnection被析构。

以2.4介绍的连接断开的情况举例。如下图中的①所示，当TcpConnection回调TcpServer的removeConnection，TcpServer会将对应的TcpConnection从其connectionSet中移出。此时TcpConnection对应的共享指针引用计数可能降为0，从而导致TcpConnection以及其拥有的channel被析构。然而在②的位置，还需要继续运行已经被析构的channel的handleEvent函数，此时很可能连core dump都不会发生，程序会继续运行，但会发生什么却是未定义的。我在没有使用tie时，调试时出现的情况是channel的数据成员莫名其妙被改动了，最后借助断言找到了问题所在。

<img src="https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/whytie.png" style="zoom:36%;" />



## 3. 性能评测

本节对muduoZ的性能进行了相关测试，通过该测试，一则可以在一定程度上检验muduoZ的性能；同时由于陈硕对muduo进行性能测试的版本已早在2011年，网上对于muduo和其它网络库的性能测试也较少（[evpp](https://github.com/Qihoo360/evpp/blob/master/docs/benchmark_throughput_vs_muduo_cn.md)在2016年左右进行了相关的性能测试，但主要是针对evpp库和其它库的比较），因此将最新版本的muduo和其它库进行一次横向比较，看看**当下**各个库之间的性能谁优谁劣；结合各个库在过去版本的测试结果，看看新版本有多少 **\"长进\"** ，想来还是挺有意思的。

### 3.1 pingpong测试

引用下[zieckey](https://github.com/zieckey)对pingpong测试的描述：

> 简单地说，ping pong 协议是客户端和服务器都实现 echo 协议。当 TCP 连接建立时，客户端向服务器发送一些数据，服务器会 echo 回这些数据，然后客户端再 echo 回服务器。这些数据就会像乒乓球一样在客户端和服务器之间来回传送，直到有一方断开连接为止。这是用来测试吞吐量的常用办法。

测试对象：

* [muduo-2.0.3](https://github.com/chenshuo/muduo/tree/v2.0.3)
* [asio-1.26.0](https://github.com/chriskohlhoff/asio/tree/asio-1-26-0)
* [libevent-2.1.12](https://github.com/libevent/libevent/tree/release-2.1.12-stable)

测试代码：

* muduo的测试代码使用陈硕的实现 https://github.com/chenshuo/muduo/tree/master/examples/pingpong

* asio的测试代码使用 [huyuguang](https://github.com/huyuguang) 的实现 https://github.com/huyuguang/asio_benchmark

  这里没有采用asio官方提供的pingpong测试代码，是因为其并没有充分发挥asio的特性从而影响了asio的性能发挥。援引下陈硕在书中的话：

  > muduo 出乎意料地比 asio 性能优越，我想主要得益于其简单的设计和简洁的代码。asio在多线程测试中表现不佳，我猜测其主要原因是测试代码只使用了一个io_service，如果改用"io_service per CPU"的话，其性能应该有所提高。我对 asio 的了解程度仅限于能读懂其代码，希望能有 asio 高手编写“io_service per CPU"的 ping pong测试，以便与muduo做一个公平的比较。

* libevent的测试代码取自 https://github.com/Qihoo360/evpp/tree/master/benchmark/throughput/libevent。这一节仅测试了libevent单线程下的性能表现，由于libevent的表现不佳，因此为公平起见，在下一节又用ibevent2 自带的性能测试程序（击鼓传花）对比了 muduo和 libevent2的性能表现。

测试环境：

* Linux ubuntu 5.4.0-144-generic x86_64 
* gcc version 7.5.0，编译时的优化统一采用 -O3

* 11th Gen Intel(R) Core(TM) i7-11800H @ 2.30GHz 

* 8-cores and 4-GB RAM are allocated in VMware 17.0.0

**测试结果：**

如下图所示，muduoZ参考了[evpp的测试方法](https://github.com/Qihoo360/evpp/tree/master)，对asio、muduo以及muduoZ在不同连接数量下，不同消息大小对应的吞吐量进行了测试。X轴对应的是连接数量，即进行pingpong消息来回发送的TCP连接数量。以连接数量为1时举例（最左侧的测试组），其对应有四组不同的消息大小，由小到大依次为4096、8192、81920、102400 KiB。通过结果可以发现，muduoZ在消息较大（81920，102400）时表现较好（超过muduo 9.1%）；在消息较小时两者没有明显区别（仅超过muduo 0.4%）。

![image-20230306153018332](https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/Performance_Test.png)

针对测试中，单个消息较大时，muduoZ优于muduo的结论，通过消融实验发现主要是与Buffer类的readFd函数实现有关，该函数负责从TCP接收缓冲区读取消息到用户缓冲区。在muduo中，采用大小为65536的extrabuf，在用户缓冲区大小不够时额外存储消息。然而当用户缓冲区剩余可用空间大于等于65536时，此时不会使用extrabuf存储数据，这就导致用户缓冲区无法进一步扩容，进而在消息较大时（>65536KiB）需要更多次数的read/write系统调用。muduoZ则并没有这方面的限制。其实还是空间和时间上的权衡，muduoZ的做法本质上是牺牲空间换取时间。

```c++
ssize_t Buffer::readFd(int fd, int* savedErrno)
{
  // saved an ioctl()/FIONREAD call to tell how much to read
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = writableBytes();
  vec[0].iov_base = begin()+writerIndex_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  // when there is enough space in this buffer, don't read into extrabuf.
  // when extrabuf is used, we read 128k-1 bytes at most.
  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = sockets::readv(fd, vec, iovcnt);
  if (n < 0)
  {
    *savedErrno = errno;
  }
  else if (implicit_cast<size_t>(n) <= writable)
  {
    writerIndex_ += n;
  }
  else
  {
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable);
  }
  return n;
}
```

在进行性能测试的过程中，发现libevent的吞吐量明显低于其它参与对比测试的网络库，因此为简洁起见，上图并未附上libevent的相关测试结果。下图为asio和libevent的性能比较，分别选用了两者较新的版本进行测试。可以发现asio的吞吐量始终优于libevent。

![image-20230306153018332](https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/per2.png)

### 3.2 击鼓传花（vs libevent2）



## 总结

如陈硕在[采访](<https://www.oschina.net/question/28_61182>)中所说：

> ”Muduo一般可以念成拼音，木铎（念：夺）。“木铎”是木舌金铃的意思，引申义是教育传播，摇铃铛以吸引行人注意。“
>
> ”Muduo的代码是写出来给人看的。Muduo的目的之一也是放在那里让人学的。”

通过学习muduo源码，我在C++的编程规范、特性使用以及对网络库的理解上都有不小的收获。在这分享一些自己的理解，抛砖引玉，若能有裨于万一，不胜之喜。
