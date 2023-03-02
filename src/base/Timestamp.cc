#include "Timestamp.h"

#include <stdio.h>

#include <string>

#include "sys/time.h"

Timestamp Timestamp::now() {
    struct timeval tm;
    if (gettimeofday(&tm, NULL) < 0) {
        fprintf(stderr, "Timestamp::gettimeofday(Line %d) failed, errno = %d\n", __LINE__, errno);
        tm.tv_sec = 0;
        tm.tv_usec = 0;
    }
    int64_t microseconds = tm.tv_sec * Timestamp::kMicrosecondsPerSecond + tm.tv_usec;
    return Timestamp(microseconds);
}

std::string Timestamp::toFormattedString() {
    char buf[64] = {0};
    char bufMicrseconds[32] = {0};
    time_t seconds = microseconds_ / Timestamp::kMicrosecondsPerSecond;
    int microseconds = static_cast<int>(microseconds_ % Timestamp::kMicrosecondsPerSecond);
    struct tm res;
    localtime_r(&seconds, &res);
    strftime(buf, sizeof buf, "%F %T.", &res);
    snprintf(bufMicrseconds, 32, "%d.", microseconds);
    return std::string(buf) + bufMicrseconds;
}

bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.microseconds() < rhs.microseconds();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.microseconds() == rhs.microseconds();
}
