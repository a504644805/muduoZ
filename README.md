muduoZ
---

<a href="https://github.com/Qihoo360/evpp/releases"><img src="https://img.shields.io/github/release/Qihoo360/evpp.svg" alt="Github release"></a>
<a href="https://travis-ci.org/Qihoo360/evpp"><img src="https://travis-ci.org/Qihoo360/evpp.svg?branch=master" alt="Build status"></a>
[![Platform](https://img.shields.io/badge/platform-%20%20%20%20Linux,%20BSD,%20OS%20X,%20Windows-green.svg?style=flat)](https://github.com/Qihoo360/evpp)
[![License](https://img.shields.io/badge/license-%20%20BSD%203%20clause-yellow.svg?style=flat)](LICENSE)
[![Project Status: Active – The project has reached a stable, usable state and is being actively developed.](http://www.repostatus.org/badges/latest/active.svg)](http://www.repostatus.org/#active)

## 0. 写在前面

这是一个基于C++11的muduo网络库。

## 1. 项目概述

在阅读完[muduo](https://github.com/chenshuo/muduo)的源码，并通过单步调试对其有一定理解后，出于学习和更深入地理解muduo，笔者重写了muduo网络库。muduoZ相比muduo主要有如下特点：

1. 去掉了Muduo库中的Boost依赖，完全使用C++标准，如使用std::function<>

2. 没有单独封装Thread，使用C++11引入的std::thread搭配lambda表达式实现工作线程，没有直接使用pthread库。类似的直接使用C++11/17的还有std::atomic，std::any，std::mutex等

3. 在异步日志中，muduo用的是fwrite+flush完成page-cache的写入，muduoZ用的则是writev。相比前者，writev的系统调用次数可以少一些，从而节省操作系统在用户态和内核态之间的切换耗时。

4. 不同于muduo直接把cookie放进FixedBuffer类，muduoZ对FixedBuffer进行了一次派生，让日志系统相关的类使用FixedBufferWithCookie。这样其它非日志系统相关的类如果想使用FixedBuffer，就不会因为也有着一样的cookie，从而和日志消息发生混淆。

5. Buffer部分Muduo库没有提供writeFd方法，本项目加入了writeFd，在处理outputBuffer剩余未发数据时交给Buffer来处理

6. 相比muduo，muduoZ只重写了核心部分。没有实现诸如[TimeZone](https://github.com/chenshuo/muduo/blob/master/muduo/base/TimeZone.cc)的辅助类。

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

关于muduo，[《Linux多线程服务端编程》](https://book.douban.com/subject/20471211//)以及众多博客都有很多介绍，在此就不重复前人发言了。挑几点我认为比较重要但很少见人说起的理解。仅是一家之言，抛砖引玉。若能有裨于万一，不胜之喜。

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

如陈硕和evpp所言，优雅的结束往往比endless的loop更难。

更确切得说：要实现优雅的结束，需要在endless loop的基础上，考虑更多的东西。介绍**状态机**



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

性能优化

1\.

2\.

### 3.1 和muduo比

单线程

![image-20230306153018332](https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/Performance_Test.png)

多线程

![image-20230306153030257](https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/Performance_Test.png)

### 3.2 和nginx比

使用了Apache Benchmark做了压测，**与nginx对比**

```
Concurrency Level:      1000
Time taken for tests:   14.852 seconds
Complete requests:      1000000
Failed requests:        0
Keep-Alive requests:    1000000
Total transferred:      118000000 bytes
HTML transferred:       14000000 bytes
Requests per second:    67330.24 [#/sec] (mean)
Time per request:       14.852 [ms] (mean)
Time per request:       0.015 [ms] (mean, across all concurrent requests)
Transfer rate:          7758.76 [Kbytes/sec] received
```

```
Concurrency Level:      1000
Time taken for tests:   48.376 seconds
Complete requests:      1000000
Failed requests:        0
Keep-Alive requests:    1000000
Total transferred:      118000000 bytes
HTML transferred:       14000000 bytes
Requests per second:    20671.60 [#/sec] (mean)
Time per request:       48.376 [ms] (mean)
Time per request:       0.048 [ms] (mean, across all concurrent requests)
Transfer rate:          2382.08 [Kbytes/sec] received
```

```
#user  nobody;
worker_processes  4;
events {
    worker_connections  10240;
}
http {
    include       /usr/local/openresty/nginx/conf/mime.types;
    default_type  application/octet-stream;
    access_log  off;
    sendfile       on;
    tcp_nopush     on;
    keepalive_timeout  65;
    server {
        listen       8001;
        server_name  localhost;
        location / {
            root   html;
            index  index.html index.html;
        }
        location /hello {
          default_type text/plain;
          echo "hello, world!";
        }
    }
}
```

## 总结

感谢硕哥，如采访所说，muduo用意

<https://www.oschina.net/question/28_61182>
