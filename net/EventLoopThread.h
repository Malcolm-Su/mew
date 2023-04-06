#ifndef MEW_EVENTLOOPTHREAD_H
#define MEW_EVENTLOOPTHREAD_H


#include <mutex>
#include <condition_variable>

#include "../base/NonCopyable.h"
#include "../base/Thread.h"
#include "EventLoop.h"

namespace mew {
class EventLoop;

class EventLoopThread : public NonCopyable {
private:
    EventLoop*  loop_ { nullptr };
    Thread      thread_ { std::bind(&EventLoopThread::RunFunc, this) };
    std::mutex  mutex_ {};
    std::condition_variable cond_ {};

private:
    void RunFunc() {
        EventLoop loop;
        {
            std::unique_lock lock { mutex_ };
            loop_ = &loop;
            cond_.notify_one();
        }

        loop_->Loop();
        {
            std::unique_lock lock { mutex_ };
            loop_ = nullptr;
        }
    }

public:
    EventLoopThread() = default;
    ~EventLoopThread() {
        thread_.Join();
        assert(loop_ == nullptr);
    }

    EventLoop* Start() {
        thread_.Start();
        EventLoop* loop = nullptr;
        {   
            std::unique_lock lock { mutex_ };
            while (loop_ == nullptr) {
                cond_.wait( lock );
            }
            loop = loop_;
        }
        return loop;
    }
};


} // namespace mew
#endif // MEW_EVENTLOOPTHREAD_H