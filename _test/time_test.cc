#include <time.h>

#include <iostream>
using namespace std;

int main() {
    char timebuf[32];
    struct tm tm;
    time_t now = time(NULL);
    localtime_r(&now, &tm);  // FIXME: localtime_r ?
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);

    cout << timebuf << endl;
}
