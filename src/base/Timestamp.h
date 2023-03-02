#ifndef MUDUOZ_SRC_BASE_TIMESTAMP_H
#define MUDUOZ_SRC_BASE_TIMESTAMP_H

#include <boost/operators.hpp>
class Timestamp : public boost::less_than_comparable<Timestamp>,
                  public boost::equality_comparable<Timestamp> {
   public:
    Timestamp() : microseconds_(0) {}
    Timestamp(int64_t microseconds) : microseconds_(microseconds) {}
    ~Timestamp() {}

    static Timestamp now();
    std::string toFormattedString();

    int64_t microseconds() { return microseconds_; }

    static const int kMicrosecondsPerSecond = 1000 * 1000;

   private:
    int64_t microseconds_;
};

bool operator<(Timestamp lhs, Timestamp rhs);
inline bool operator==(Timestamp lhs, Timestamp rhs);

#endif