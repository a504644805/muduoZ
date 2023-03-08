// thread example
#include <sys/syscall.h>

#include <iostream>  // std::cout
#include <thread>    // std::thread

#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif
inline int getTid() {
    return static_cast<pid_t>(syscall(SYS_gettid));
}
void foo() {
    // do stuff...
    int pid = getpid();
    std::cout << "pid=" << pid << ",tid=" << std::this_thread::get_id() << ",first thread foo\n";
    std::cout << "gettid is " << getTid() << std::endl;
}

void bar(int x) {
    // do stuff...
    int pid = getpid();
    std::cout << "pid=" << pid << ",tid=" << std::this_thread::get_id() << ",second thread bar\n";
    std::cout << "gettid is " << getTid() << std::endl;
}

// c++之std::thread多线程的使用和获取pid/tid
int main() {
    std::thread first(foo);      // spawn new thread that calls foo()
    std::thread second(bar, 0);  // spawn new thread that calls bar(0)

    int pid = getpid();
    std::cout << "pid=" << pid << ",tid=" << std::this_thread::get_id() << ",main, foo and bar now execute concurrently...\n";
    std::cout << "gettid is " << getTid() << std::endl;

    // synchronize threads:
    first.join();   // pauses until first finishes
    second.join();  // pauses until second finishes

    std::cout << "foo and bar completed.\n";
    system("pause");
    return 0;
}
