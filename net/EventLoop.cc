#include <utility>

#include <unistd.h>
#include <sys/eventfd.h>
#include <pthread.h>
#include <signal.h>

#include "EventLoop.h"
#include "Event.h"
#include "../base/Thread.h"
#include "TaskQueue.h"

using namespace mew;

namespace {

class ignore_sigpip {
public:
    ignore_sigpip() {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

ignore_sigpip initObj;

}  // namespace

EventLoop::EventLoop()
    : loop_thread_tid_( current_thread::GetTid() ),
      poller_(new EventPoller()),
      task_queue_(new TaskQueue(this)),
      timer_queue_(new TimerQueue(this))
{}

EventLoop::~EventLoop() {
    if (running_) running_ = false;

}

void EventLoop::Loop() {
    assert(IsInLoopThread());
    running_ = true;
    while (running_) {
        active_events_.clear();
        poller_->Poll(active_events_);
        for (const auto& event : active_events_) {
            event->OnEvent();
        }

    }
    running_ = false;
}


void EventLoop::AppendTask(LoopTask task) {
    task_queue_->Append(std::move(task));
}

void EventLoop::ToRun(LoopTask task) { 
    if (IsInLoopThread()) {   
        task(); 
    } else {
        AppendTask(std::move(task));  
    } 
}

