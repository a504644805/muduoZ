#ifndef MUDUOZ_NET_CURRENTTHREAD_H
#define MUDUOZ_NET_CURRENTTHREAD_H
#include <sys/syscall.h>
#include <unistd.h>
namespace muduoZ {
namespace CurrentThread {

extern __thread int t_cachedTid;

inline int getTid() {
    if (__builtin_expect(t_cachedTid != -1, 1))
        ;
    else
        t_cachedTid = static_cast<pid_t>(syscall(SYS_gettid));
    return t_cachedTid;
}

}  // namespace CurrentThread
}  // namespace muduoZ

#endif