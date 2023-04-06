#ifndef MEW_TIMER_H
#define MEW_TIMER_H

#include <functional>

#include "../base/NonCopyable.h"
#include "../time/Time.h"
#include "../base/Callback.h"

using mew::time::Timestamp;

namespace mew {

class Timer : public NonCopyable {
public:

private:
    Timestamp          expiration_time_; 
    TimerTask          task_;
    bool               repeat_;
    time::milliseconds interval_; 

public:
    Timer(Timestamp expiration_time, TimerTask&& cb, time::milliseconds interval)
        : expiration_time_(expiration_time),
          task_(std::move(cb)),
          repeat_(interval.count() > 0) ,
          interval_(interval)
    {}

public:
    void Restart() {
        expiration_time_ = Timestamp::after(interval_);
    }

    void Run() const {
        task_();
    }

    Timestamp GetExpirationTime() const { return expiration_time_; }
    bool IsRepeat() const { return repeat_; }

};

} //  namespace mew
#endif // MEW_TIMER_H