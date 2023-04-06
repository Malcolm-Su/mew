#include "Event.h"

#include "EventLoop.h"

namespace mew {

Event::Event(EventLoop *loop, int fd)
    : loop_(loop),
      fd_(fd)
{}

Event::~Event() { 
    if (state_ == kMonitored) {
        AbandonAndUpdate();
    }
    if (fd_ > 0) ::close(fd_); 
}


void Event::OnEvent() {
    std::shared_ptr<void> guard = nullptr;
    if (tied_) 
        guard = tie_.lock();

    if (recv_events_ & POLLNVAL) {
        ERROR("Event::OnEvent POLLNVAL");
    }
    
    if (recv_events_ & (EPOLLERR | POLLNVAL)) {
        if (error_task_) error_task_();
    }

    if (recv_events_ & (kRead | EPOLLRDHUP)) {
        if (read_task_) read_task_();
    } 

    if (recv_events_ & kWrite) {
        if (write_task_) write_task_();
    }
}

void Event::Update() {
    if (loop_) loop_->ToUpdate(this);
}


void Event::AbandonAndUpdate() { 
    if (state_ == kFree) return;

    state_ = kAbandoned; 
    Update();
}

void Event::EnableAndUpdate(uint32_t to_enable) {
    interested_events_ |= to_enable;
    Update();
}

void Event::DisableAndUpdate(uint32_t to_disable) {
    interested_events_ &= ~to_disable;
    Update();
}

void Event::DisableAllAndUpdate() {
    interested_events_ = 0;
    Update();
}





} // namespace mew