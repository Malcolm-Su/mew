#ifndef MEW_EVENT_H_
#define MEW_EVENT_H_

#include "../base/Callback.h"

#include <sys/epoll.h>
#include <sys/poll.h>
#include <unistd.h>

#include <functional>

#include "../log/Log.h"


namespace mew {
class EventLoop;


class Event {
public:
    constexpr static uint32_t kRead  = EPOLLIN | EPOLLPRI;
    constexpr static uint32_t kWrite = EPOLLOUT;
    constexpr static uint32_t kEdgeTriggered = EPOLLET;

    enum State { kFree, kMonitored, kAbandoned };

private:
    EventLoop *loop_;
    int fd_;                   
    uint32_t interested_events_ = 0;
    uint32_t recv_events_ = 0;
    State    state_  = kFree;

    std::weak_ptr<void> tie_;
    bool tied_ = false;

    EventTask read_task_;
    EventTask write_task_;
    EventTask error_task_;
    
public:
    Event(EventLoop *loop, int fd);

    ~Event();

public: 
    void SetReadTask(EventTask task)  { read_task_  = std::move(task); }
    void SetWriteTask(EventTask task) { write_task_ = std::move(task); }
    void SetErrorTask(EventTask task) { error_task_ = std::move(task); }

    void OnEvent();

    int      GetFd() const { return fd_; }
    uint32_t GetInterestedEvents() const { return interested_events_; }
    uint32_t GetReceivedEvents() const { return recv_events_; }
    State    GetState() const { return state_; }

    bool IsReadEnabled() const { return interested_events_ & kRead; }
    bool IsWriteEnabled() const { return interested_events_ & kWrite; }

    void SetReceivedEvents(uint32_t ev) { recv_events_ = ev; }
    void SetInterestedEvents(uint32_t ev) { interested_events_ = ev; }
    void SetState(State new_state) { state_ = new_state; }
    void SetTie(std::shared_ptr<void> tie) { tie_ = tie; tied_ = true; }

    void Update();

    void AbandonAndUpdate();
    
    void EnableAndUpdate(uint32_t to_enable);

    void DisableAndUpdate(uint32_t to_disable);

    void DisableAllAndUpdate();


}; // class Event


} // namespace mew
#endif // MEW_EVENT_H_


// static void test() {
   
    
// }