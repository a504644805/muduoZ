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

3. We use writev instead of fwrite for less system call.

4. We don\'t put cookie\_ inside FixedBuffer, so others can freely use FixedBuffer without having same cookie as AsyncLogging, which may confuse us

5. Buffer部分Muduo库没有提供writeFd方法，本项目加入了writeFd，在处理outputBuffer剩余未发数据时交给Buffer来处理

6. 相比muduo，muduoZ只重写了核心部分。没有实现诸如[TimeZone](https://github.com/chenshuo/muduo/blob/master/muduo/base/TimeZone.cc)的辅助类。

### 1.1 总览

muduo是由[chen shuo](https://github.com/chenshuo)所写基于Reactor架构的高性能C++网络库。

通过阅读源码，我将构成muduo的框架划分为三块内容：

1）由EventLoop、Poller、Channel构成的Reactor框架

2）由TcpServer、Acceptor、TcpConnection构成的Server三件套

3）由TcpClient、Connector、(TcpConnection)构成的Client三件套

整体的UML类图如下所示（为突出重点，只画出核心类及各类的核心成员）。其中由EventLoop、Poller、Channel构成的Reactor框架是muduo的核心，EventLoop的loop函数会循环往复地进行

1.调用poll/epoll （由Poller封装)。

2.处理I/O事件

Server/Client则通过Acceptor/Connector来进行连接的接收/发起。其中Server可以管理多条连接（TcpConnection），Client则最多只对应一条连接。TcpConnection、Acceptor、Connector都会通过Channel来接入Reactor框架（2.1 对如何接入进行了介绍）。

![image-20230307201306560](https://raw.githubusercontent.com/a504644805/resources/master/muduoZ/UML_CLASS_GRAPH.png)

## 2. 几点心得

关于muduo，[《Linux多线程服务端编程》](https://book.douban.com/subject/20471211//)以及众多博客都有很多介绍，在此就不重复前人发言了。挑几点我认为比较重要但很少见人说起的理解。仅是一家之言，抛砖引玉。若能有裨于万一，不胜之喜。

### 2.1 Channel==黏合剂

要深刻理解总览中提到的三部分间的关联，关键是把握Channel类作为**\"黏合剂\"**的本质。

只看Channel，会发现其负责某个socket fd的事件分发（handleEvent函数），却又不负责socket fd的生命周期，好像是个非常普通又有些奇怪的类。

但当我们从整体来观察，就会发现Channel和Poller属于聚合关系；和Acceptor，Connector，TcpConnection属于组合关系；EventLoop也用到了Channel类来进行唤醒操作，简直无所不在。这本质上是因为muduo采用Channel到Poller的注册和注销机制实现I/O事件的动态增删。

实现机制如下：

1. Channel及“Channel负责的socket fd”的生命周期和Channel的拥有者一致（拥有者包括 Acceptor，Connector，TcpConnection etc. )

2. Channel的拥有者负责Channel的创建、配置、注册、注销这一系列动作。

以TcpConnection为例进行说明：

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

【注】上述代码来自muduoZ，muduo的做法也类似。



### 2.2 如何优雅地结束

如陈硕和evpp所言，优雅的结束往往比endless的loop更难。

更确切得说：要实现优雅的结束，需要在endless loop的基础上，考虑更多的东西。介绍**状态机**



### 2.3 一条日志消息都别想跑



### 2.4 别被回调绕晕了

这个本来没打算写的，无意中看见知乎的一个评论，仔细想想初看muduo确实花了一点时间捋清楚里面的回调逻辑，所以一并写了吧



### 2.5 为何要tie

在重写muduo时，我有一条原则：遇见自己难以理解的手法，在查阅资料无果后，先标注，然后按照自己的理解去写而不是原样照搬。这虽然让我在吃了不少苦头（调试程序的时间更久了，刚写完作性能测试发现比muduo差了三倍，花了几天才找到所有病症），但收获也不少，理解为何要tie就是收获之一。

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
