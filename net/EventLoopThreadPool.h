#ifndef MEW_EVENTLOOPTHREADPOOL_H
#define MEW_EVENTLOOPTHREADPOOL_H

#include <vector>
#include <memory>

#include "../base/NonCopyable.h"


namespace mew {

class EventLoopThread;
class EventLoop;

class EventLoopThreadPool : public NonCopyable {
public:
    using Threads = std::vector<std::unique_ptr<EventLoopThread>>;
    using Loops   = std::vector<EventLoop*>;

private:
    EventLoop*  base_loop_;   
    Threads     threads_;
    Loops       loops_;

    int thread_num_;
    int next_;

public:
    EventLoopThreadPool(EventLoop* loop);
    ~EventLoopThreadPool();
   
    void SetThreadNum(int thread_num) {
        thread_num_ = thread_num;
    }

    void Start();
    EventLoop* GetNextLoop();

};

} // namespace mew
#endif // MEW_EVENTLOOPTHREADPOOL_H