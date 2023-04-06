#ifndef MEW_TIME_H
#define MEW_TIME_H

#include <string>
#include <ctime>
#include <chrono>

#include <sys/time.h>


namespace mew::time {
using namespace std::chrono;

using Rep = int64_t;

// 保留类似标准库的 lower case naming convention

class Timestamp {
private:
    Rep us_since_epoch_;
public:
    Timestamp(): us_since_epoch_(0)
    {}

    Timestamp(Rep us): us_since_epoch_(us)
    {}

    Timestamp(const Timestamp &other) = default;
    Timestamp& operator=(const Timestamp &other) = default;
    
    Timestamp(Timestamp &&other) = default;
    Timestamp& operator=(Timestamp &&other) = default;

    ~Timestamp() = default;

public:
    std::string to_string(const char *format = "%Y-%m-%d %H:%M:%S") {
        char timestamp[32];
        time_t ticks = duration_cast<seconds>(microseconds(us_since_epoch_)).count();
        struct tm *ptm = localtime(&ticks);
        // size_t timestamp_len = 
        strftime(timestamp, sizeof(timestamp), format, ptm);
        return std::string(timestamp);
    }

    bool operator<(const Timestamp &rhs) const { 
        return us_since_epoch_ < rhs.us_since_epoch_;
    }

    bool operator==(const Timestamp &rhs) const { 
        return us_since_epoch_ == rhs.us_since_epoch_;
    }

    bool operator>(const Timestamp &rhs) const { 
        return us_since_epoch_ > rhs.us_since_epoch_;
    }
    microseconds operator-(const Timestamp &rhs) const { 
        return microseconds( us_since_epoch_ - rhs.us_since_epoch_ ) ;
    }


public:
    static Timestamp now() {
        struct timeval tv;
        if (gettimeofday(&tv, NULL) == -1) {
            tv.tv_sec = ::time(NULL);
            tv.tv_usec = 0;
        }
        return Timestamp(tv.tv_sec*1000*1000 + tv.tv_usec);
    }

    template<typename Duration>
    static Timestamp after(Duration delta) {
        return Timestamp( now().us_since_epoch_ + duration_cast<microseconds>(delta).count() );
    }

    template <typename Duration>
    static Timestamp from(Timestamp from, Duration delta) {
        return Timestamp( from.us_since_epoch_ + microseconds( delta ).count() );
    }
    

}; // class Timestamp



} // namespace mew::time





#endif // MEW_TIME_H



