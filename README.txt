muduoZ is a rewrite of muduo, the difference(aim) between muduo are follows:
1.optimize the efficiency
2.c++11
3.don't rely boost 


For 3:
-boost::noncopyable
https://fzheng.me/2016/11/20/cpp_noncopyable_class/


#include <mutex>
// C++11 has offered mutex class and std::lock_guard (we can use them for RAII
// of mutex) https://en.cppreference.com/w/cpp/thread/mutex

#include <condition_variable>
// C++11 has offered condition_variable class


Solution for unflushed buffer
Pre:
1. fwrite sometimes has less system call compared with write, because fwrite has buffer in user space.
https://blog.csdn.net/liuxiao723846/article/details/120925213
2. After calling fwrite, data may still be in user space, in this case ,if process dump, OS won't put data into disk. After flush, data are transfered to page cache, data will be put into disk even if process dump.
3. fwrite_unlocked are faster than fwrite, we can use it if the file is accessed by only one thread.
https://stackoverflow.com/questions/72643109/what-is-unlocked-stdio-in-c
Sol:
1. First flush then Destruct buffer !!! So cookie_=bufferEnd means invalid buffer
2. Because of 1, buffer has not been flushed has cookie_=bufferStart, we can find them in core file:
    (1) gdb a.out core
    (2) info address muduoZ::detail::FixedBuffer<4000000>::bufferStart #get the virtual address of bufferStart
    (3) xxd core #search the virtual address

