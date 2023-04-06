#include <vector>

#include "EventPoller.h"

#include <assert.h>
#include <string.h>
#include <sys/epoll.h>

#include "Event.h"
#include "../log/Log.h"

using namespace mew;



void EventPoller::LoadActiveEvents(int eventnums, Events& events) {
    for (int i = 0; i < eventnums; ++i) {
        Event* ptr = static_cast<Event*>(ep_evs_[i].data.ptr);
        ptr->SetReceivedEvents(ep_evs_[i].events);
        events.emplace_back(ptr);
    }
    if ( eventnums == static_cast<int>( ep_evs_.size() ) ) {
        ep_evs_.resize(eventnums * 2);
    }
}

/////////////////////////////////////////////////////////
///////// public: 

EventPoller::EventPoller()
    : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      ep_evs_(DEFAULT_EPOLLEVENT_NUM), 
      events_()
{
    assert(epollfd_ != -1);
}

EventPoller::~EventPoller() {
    ::close(epollfd_);
}


void EventPoller::Poll(Events& events) {
    int eventnums = Wait_();
    LoadActiveEvents(eventnums, events);
}


/**
 * @brief 从 EventPoller 中移除关于 event 的监听
 * @attention 需要保证 event 的 state 是 kMonitored 或者 kAbandoned 的
 * @param event 
 */
void EventPoller::Remove(Event* event) {
    event->AbandonAndUpdate();
}


/**
 * @brief 根据 event 的状态来选择 注册, 删除, 更新
 * @param event 
 */
void EventPoller::Update(Event* event) {
    int op = 0;
    int fd = event->GetFd();
    Event::State state = event->GetState(); 

    switch (state) {
        case Event::State::kFree: 
            assert(events_.find(fd) == events_.end());

            op = EPOLL_CTL_ADD;
            events_[fd] = event;
            event->SetState(Event::State::kMonitored);
            break;

        case Event::State::kMonitored:
            assert(events_.find(fd) != events_.end());
            assert(events_[fd] == event);

            op = EPOLL_CTL_MOD;
            break;

        case Event::State::kAbandoned:
            assert(events_.find(fd) != events_.end());

            op = EPOLL_CTL_DEL;
            event->SetState(Event::State::kFree);
            events_.erase(fd);
            break;
    }

    struct epoll_event ep_ev;
    ep_ev.data.ptr = static_cast<void*>(event);
    ep_ev.events = event->GetInterestedEvents();

    if (epoll_ctl(epollfd_, op, event->GetFd(), &ep_ev) < 0) {
        ERROR( "EventPoller::Update epoll_ctl failed, EPOLL_CTL code:%d, fd:%d, epev: %u", op, event->GetFd(), event->GetInterestedEvents() );
        perror("error: ");
    }
}

