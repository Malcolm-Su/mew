#ifndef MEW_TASK_QUEUE_H_
#define MEW_TASK_QUEUE_H_

#include <sys/eventfd.h>

#include <memory>

#include "Event.h"

#include "EventLoop.h"


namespace mew {

class TaskQueue {
public:
    using TaskList = std::vector<LoopTask>;
private:
    EventLoop *loop_;
    std::unique_ptr<Event> event_;
    TaskList task_list_;
    
    bool is_handling_ = false;

    std::mutex mutex_;

private:
    void ReadFd_();
    void OnRead_();

public:
    TaskQueue(EventLoop *loop);
    ~TaskQueue();

public:
    void Append(LoopTask task);

}; // class TaskQueue


} // namespace mew
#endif // MEW_TASK_QUEUE_H_