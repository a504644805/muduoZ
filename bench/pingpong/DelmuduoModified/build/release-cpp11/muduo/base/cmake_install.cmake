# Install script for directory: /home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/build/release-cpp11/release-install-cpp11")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/build/release-cpp11/lib/libmuduo_base.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/muduo/base" TYPE FILE FILES
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/AsyncLogging.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/Atomic.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/BlockingQueue.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/BoundedBlockingQueue.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/Condition.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/CountDownLatch.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/CurrentThread.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/Date.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/Exception.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/FileUtil.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/GzipFile.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/LogFile.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/LogStream.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/Logging.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/Mutex.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/ProcessInfo.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/Singleton.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/StringPiece.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/Thread.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/ThreadLocal.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/ThreadLocalSingleton.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/ThreadPool.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/TimeZone.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/Timestamp.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/Types.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/WeakCallback.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/copyable.h"
    "/home/xy/projects/muduoZ/bench/pingpong/muduoModified/muduo/base/noncopyable.h"
    )
endif()

