#ifndef MEW_TCPCONNECTION_H
#define MEW_TCPCONNECTION_H

#include <memory>
#include <any>

#include "Event.h"
#include "../base/NonCopyable.h"
#include "../base/Callback.h"
#include "../utils/Buffer.h"
#include "../http/HttpContent.h"
#include "../time/Time.h"

namespace mew {


class EventLoop;


class TcpConnection : public std::enable_shared_from_this<TcpConnection>, NonCopyable {
public:
    enum class State {
        kConnecting,
        kConnected,
        kDisconnecting,     
        kDisconnected
    };

private:
    EventLoop* loop_;
    int connection_id_;
    State state_;
    std::unique_ptr<Event> event_;

    Buffer input_buffer_;
    Buffer output_buffer_;
    // std::any context_;
    std::shared_ptr<void> context_;
    
    Timestamp last_active_time_;
    time::milliseconds close_timeout_ = time::seconds(30);
    bool shutdown_flag_ = false;

    TcpConnectionTask connection_task_;
    TcpMessageTask    message_task_;
    TcpCloseTask      close_task_;

private:
    void SetupCloseTimer();


public:
    TcpConnection(EventLoop* loop, int connfd, int id);
    ~TcpConnection();

public:
    void SetConnectionTask(const TcpConnectionTask& task) { 
        connection_task_ = task;
    }

    void SetConnectionTask(TcpConnectionTask&& task) { 
        connection_task_ = std::move(task);
    }

    void SetMessageTask(const TcpMessageTask& task) {
        message_task_ = task;
    }

    void SetMessageTask(TcpMessageTask&& task) {
        message_task_ = std::move(task);
    }

    void SetCloseTask(const TcpCloseTask& task) {
        close_task_ = task;
    }

    void SetCloseTask(TcpCloseTask&& task) {
        close_task_ = std::move(task);
    }


    void Shutdown();  
    void RealShutdown();
    bool IsShutdown() const { return shutdown_flag_; }  
    int  GetErrno() const;

    void OnConnection();


    void OnClose();

    int FlushOutputBuffer();

    void OnMessage();  // Eventloop callback

    void OnWrite();    // Eventloop callback

    void OnError();    // Eventloop callback


    void Send(Buffer* buffer);
    void Send(const string& str);
    void Send(const char* message, int len);
    void Send(const char* message) { Send(message, static_cast<int>(strlen(message))); }
    void UpdateLastActiveTime(Timestamp now) { last_active_time_ = now; }

    Timestamp GetLastActiveTime() const { return last_active_time_; }
    int GetFd() const { return event_->GetFd(); }
    int GetId() const { return connection_id_; }
    EventLoop* GetLoop() const { return loop_; }

    template <typename T>
    std::shared_ptr<T> GetContext() { return std::static_pointer_cast<T>(context_); }

    template <typename T>
    void SetContext(std::shared_ptr<T> ctx) { context_ = ctx; }

};

typedef std::shared_ptr<TcpConnection> TcpconnectionPtr;

} // namespace mew
#endif // MEW_TCPCONNECTION_H

