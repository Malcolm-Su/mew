#include "EventLoopThreadPool.h"

#include <memory>

#include "EventLoopThread.h"

using namespace mew;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop)
    : base_loop_(loop),
      thread_num_(0),
      next_(0) 
{}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::Start() {
    for ( int i = 0; i < thread_num_; ++i ) {
        EventLoopThread* ptr = new EventLoopThread();
        threads_.emplace_back( std::unique_ptr<EventLoopThread>(ptr) );
        loops_.emplace_back( ptr->Start() );
    }
    INFO("EventLoopThreadPool: %d of threads started!", thread_num_);
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
    EventLoop* ret = base_loop_;
    if ( !loops_.empty() ) {
        ret = loops_[next_++];
        if ( next_ == static_cast<int>( loops_.size() ) ) next_ = 0;
    }

    return ret;
}
