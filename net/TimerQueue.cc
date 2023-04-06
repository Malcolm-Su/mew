
#include <assert.h>
#include <sys/timerfd.h>

#include <string.h>

#include "TimerQueue.h"
#include "Event.h"
#include "EventLoop.h"

namespace mew {



void TimerQueue::Insert(Timer* timer) {
    bool reset_instantly = false;
    if (timers_.empty() || timer->GetExpirationTime() < timers_.begin()->first ) {
        reset_instantly = true;
    }

    timers_.emplace(TimerPair(timer->GetExpirationTime(), timer));
    
    if (reset_instantly)
        ResetTimerFd(timer);
}


void TimerQueue::OnReadTimerFd() {
    uint64_t read_byte;
    ssize_t readn = ::read(timerfd_, &read_byte, sizeof(read_byte));
    if (readn != sizeof(read_byte)) {
        ERROR("TimerQueue::read_timer_fd read_size < 0");
    }
}


void TimerQueue::OnRead() {
    OnReadTimerFd();
    Timestamp expiration_time(Timestamp::now());

    active_timers_.clear();    
    auto end = timers_.lower_bound(TimerPair(Timestamp::now(), reinterpret_cast<Timer*>(UINTPTR_MAX)));
    active_timers_.insert(active_timers_.end() , timers_.begin(), end);
    timers_.erase(timers_.begin(), end);
    
    for (const auto& timerpair : active_timers_) {
        timerpair.second->Run();
    } 
    ResetTimers();
}


void TimerQueue::ResetTimers() {
    for ( auto& timerpair: active_timers_ ) {
        if ( timerpair.second->IsRepeat() ) {
            auto timer = timerpair.second;
            timer->Restart();
            Insert(timer);
        } else {
            delete timerpair.second;
        }
    } 

    if (!timers_.empty()) {
        ResetTimerFd(timers_.begin()->second);
    }
}


TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerfd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
      event_( new Event(loop_, timerfd_) ) 
{
    event_->SetReadTask( std::bind(&TimerQueue::OnRead, this) );
    event_->EnableAndUpdate(Event::kRead);
    INFO("TimerQueue: initialized");
}


TimerQueue::~TimerQueue() {
    loop_->ToRemove(event_.get());

    ::close(timerfd_);
    for (const auto& timerpair : timers_) {
        delete timerpair.second;
    } 
    for (const auto& timerpair : active_timers_) {
        delete timerpair.second;
    } 
}


void TimerQueue::ResetTimerFd(Timer* timer) {
    struct itimerspec new_spec;
    memset(&new_spec, 0, sizeof(new_spec));

    int64_t micro_seconds_dif = (timer->GetExpirationTime() - Timestamp::now()).count();
    if ( micro_seconds_dif < 100 ) {
        micro_seconds_dif = 100;
    }

    constexpr static int64_t us_per_sec = 1000*1000;

    new_spec.it_value.tv_sec = static_cast<time_t>(
        micro_seconds_dif / us_per_sec
    );

    new_spec.it_value.tv_nsec = static_cast<long>(
        (micro_seconds_dif % us_per_sec) * 1000
    );

    int ret = ::timerfd_settime(timerfd_, 0, &new_spec, NULL);
    assert(ret != -1);
}


} // namespace mew