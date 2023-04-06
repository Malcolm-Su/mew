#include "TaskQueue.h"

#include "EventLoop.h"

namespace mew {

void TaskQueue::ReadFd_() {
    uint64_t read_one_byte = 1;
    ssize_t read_size = ::read(event_->GetFd(), &read_one_byte, sizeof(read_one_byte));
    assert(read_size == sizeof(read_one_byte));
}

void TaskQueue::OnRead_() {
    ReadFd_();
    TaskList tasks;
    is_handling_ = true;
    {
        std::unique_lock lock { mutex_ };
        tasks.swap(task_list_);
    }

    for (const auto& task : tasks) {
        task();
    }
    is_handling_ = false;

}

TaskQueue::TaskQueue(EventLoop *loop)
    : loop_(loop),
      event_( new Event( loop, ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC) ) )
{
    assert(event_->GetFd() != -1);
    event_->SetReadTask(std::bind(&TaskQueue::OnRead_, this));
    event_->EnableAndUpdate(Event::kRead);
}

TaskQueue::~TaskQueue() {
    event_->AbandonAndUpdate();
}


void TaskQueue::Append(LoopTask task) {
    {  
        std::unique_lock lock { mutex_ };
        task_list_.emplace_back(std::move(task));
    }

    if (!loop_->IsInLoopThread() || is_handling_) {
        uint64_t write_one_byte = 1;  
        ssize_t write_size = ::write(event_->GetFd(), &write_one_byte, sizeof(write_one_byte));
        (void) write_size;
        assert(write_size == sizeof(write_one_byte));
    } 
}




} // namespace mew
