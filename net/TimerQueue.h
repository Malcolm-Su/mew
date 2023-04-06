#ifndef MEW_TIMERQUEUE_H
#define MEW_TIMERQUEUE_H

#include <unistd.h>
#include <sys/timerfd.h>

#include <set>
#include <vector>
#include <memory>
#include <utility>
#include <functional>

#include "Timer.h"
#include "../base/NonCopyable.h"
#include "../log/Log.h"
#include "../base/Callback.h"

namespace mew {

class EventLoop;
class Event;

class TimerQueue : public NonCopyable {
public:
    using TimerPair    = std::pair<Timestamp, Timer*>;
    using TimersSet    = std::set<TimerPair>  ; 
    using ActiveTimers = std::vector<TimerPair>;

private:
    EventLoop *loop_;
    int timerfd_;
    std::unique_ptr<Event> event_; 

    TimersSet timers_;
    ActiveTimers active_timers_;

private:
    void OnReadTimerFd();
    void ResetTimerFd(Timer* timer);
    void ResetTimers();
    void OnRead();
    

public:
    TimerQueue(EventLoop *loop);
    ~TimerQueue();

public:
    void Insert(Timer* timer);
    Event* GetEvent() const { return event_.get(); }
};

} // namesapce mew

#endif  // MEW_TIMERQUEUE_H
