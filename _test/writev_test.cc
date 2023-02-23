#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <iostream>
using namespace std;

int main() {
    int fd = open("./file1.txt", O_RDONLY);
    assert(fd >= 0);

    char extrabuf[10];
    char extrabuf2[65536];
    struct iovec vec[2];
    vec[0].iov_base = extrabuf;
    vec[0].iov_len = sizeof extrabuf;
    vec[1].iov_base = extrabuf2;
    vec[1].iov_len = sizeof extrabuf2;
    const int iovcnt = 2;
    const ssize_t n = readv(fd, vec, iovcnt);
    cout << "read " << n << " bytes" << endl;
    cout << extrabuf[9]<<endl;
    cout << extrabuf2[2]<<endl;
    return 0;
}
