#ifndef MEW_ACCEPTOR_H
#define MEW_ACCEPTOR_H


#include <functional>
#include <memory>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "../base/NonCopyable.h"

namespace mew {


class EventLoop;
class Address;
class Event;

class Acceptor : public NonCopyable {
public:
    using NewConnectionTask = std::function<void (int, const Address&)>;

private:
    EventLoop* loop_;
    int listenfd_;
    int idlefd_;    // 防止 busy loop
    std::unique_ptr<Event> event_;
    NewConnectionTask new_connection_task_; 

public:
    Acceptor(EventLoop* loop, const Address& address);
    ~Acceptor();

    void Bind(const Address& address); 
    void Listen();
    void OnNewConnection();

    int SetSockeOptKeepAlive(int fd) {
        int option_val = 1;
        return ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,
                        &option_val, static_cast<socklen_t>(sizeof(option_val)));
    }

    int SetSockOptReuseaddr(int fd) {
        int option_val = 1;
        return ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                        &option_val, static_cast<socklen_t>(sizeof(option_val)));
    }

    int SetSockOptTcpNoDelay(int fd) {
        int option_val = 1;
        return ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                        &option_val, static_cast<socklen_t>(sizeof(option_val)));
        
    }
    
    void SetNewConnectionTask(const NewConnectionTask& handler) {
        new_connection_task_ = handler;
    }

    void SetNewConnectionTask(NewConnectionTask&& handler) {
        new_connection_task_ = std::move(handler);
    }

};


} // namespace mew
#endif // MEW_ACCEPTOR_H