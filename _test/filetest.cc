#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
using namespace std;
using namespace std;

void* threadFunc(void*) {
    std::string filename = "Log.";
    int fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_APPEND | O_EXCL | O_CLOEXEC, S_IRUSR | S_IWRITE);
    if (fd < 0) {
        fprintf(stderr, "AsyncLogging::threadFunc(Line %d): open failed, errno = %d\n", __LINE__, errno);
        assert(fd >= 0);  // FIXME:terminate the process.(now is Debug version, we simply use assert to terminate)
    }
    while (1)
        ;
}
int main() {
    pthread_t tid1;
    pthread_create(&tid1, NULL, threadFunc, NULL);
    while (1)
        ;
    return 0;
}
