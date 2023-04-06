#ifndef MEW_EVENTLOOP_H
#define MEW_EVENTLOOP_H

#include <memory>


#include "TimerQueue.h"
#include "../base/NonCopyable.h"
#include "EventPoller.h"
#include "Timer.h"
#include "Event.h"



using namespace mew::time;

namespace mew {
class TaskQueue;

class EventLoop : public NonCopyable {
public:
    using EventList = std::vector<Event*>;
    using TaskList  = std::vector<LoopTask>;

private:
    bool  running_ = false;
    pid_t loop_thread_tid_; 
    
    std::unique_ptr<EventPoller> poller_;
    std::unique_ptr<TaskQueue>   task_queue_;
    std::unique_ptr<TimerQueue> timer_queue_;

    EventList active_events_ {};

private:
    void ToAddTimer_(Timestamp timestamp, TimerTask &&task, mew::time::milliseconds interval) {
        Timer *timer { new Timer(timestamp, std::move(task), interval) };
        ToRun( std::bind( &TimerQueue::Insert, timer_queue_.get(), timer ) );
    }
    
public:
    EventLoop();
    ~EventLoop();

public:
    void ToRun(LoopTask task);

    void ToRunAt(Timestamp timestamp, TimerTask&& task) {
        ToAddTimer_(timestamp, std::move(task), 0ms);
    }

    /**
     * @brief 让 loop 去添加 timer
     * @tparam Duration 兼容标准库 chrono::duration 的类型
     * @param wait_time 等待时间
     * @param task timer 的任务
     */
    template <typename Duration>
    void ToRunAfter(Duration wait_time, TimerTask&& task) {
        Timestamp expiration_time { Timestamp::after(wait_time) }; 
        ToAddTimer_(expiration_time, std::move(task), 0ms);
    }

    /**
     * @brief 让 loop 去添加 timer
     * @tparam Duration 兼容标准库 chrono::duration 的类型
     * @param interval 间隔时间
     * @param task timer 的任务
     */
    template <typename Duration>
    void ToRunEvery(Duration interval, TimerTask&& task) {
        Timestamp expiration_time { Timestamp::after(interval) }; 
        ToAddTimer_(expiration_time, std::move(task), interval);
    }

    bool IsInLoopThread() { return current_thread::GetTid() == loop_thread_tid_; }

    void ToUpdate(Event *event) { 
        if (IsInLoopThread())
            poller_->Update(event); 
        else 
            AppendTask( [this, event](){ this->poller_->Update(event); } );
    }
    
    void ToRemove(Event* event) { 
        if (IsInLoopThread())
            poller_->Remove(event); 
        else 
            AppendTask( [event,this](){ this->poller_->Remove(event); } );
    }

    void AppendTask(LoopTask task);

    void Loop();
};


} // namespace mew
#endif // MEW_EVENTLOOP_H