#ifndef MEW_EVENTPOLLER_H_
#define MEW_EVENTPOLLER_H_

#include <vector>
#include <map>

#include <sys/epoll.h>

#include "../base/NonCopyable.h"
#include "Event.h"


static constexpr int DEFAULT_EPOLLEVENT_NUM = 16;

namespace mew {

class EventPoller : public NonCopyable {
public: 
    using EpollEvents = std::vector<struct epoll_event>;
    using Events      = std::vector<Event*>;

private: 
    using EventMap = std::map<int, Event*>;

    int             epollfd_;
    EpollEvents     ep_evs_;
    EventMap        events_;       
                    // not mananging the lifetime of the events

private:
    int Wait_() { 
        return epoll_wait( epollfd_, &*ep_evs_.begin(), static_cast<int>( ep_evs_.size() ), -1 ); 
    }
    
    void LoadActiveEvents(int eventnums, Events& events); 

public:
    EventPoller();
    ~EventPoller();
    
public:
    void Update(Event *event);
    
    void Remove(Event* event);

    /**
     * @brief 拉取有活动的 epoll_event, 放入 events 中
     * @param events 
     */
    void Poll(Events& events);
};


} // namespace mew
#endif // MEW_EVENTPOLLER_H_