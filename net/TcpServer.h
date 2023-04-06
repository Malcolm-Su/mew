#ifndef MEW_TCPSERVER_H
#define MEW_TCPSERVER_H


#include <map>
#include <memory>


#include "../base/NonCopyable.h"
#include "EventLoop.h"
#include "TcpConnection.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"

namespace mew {



class Address;

class TcpServer : public NonCopyable {
private:
    typedef std::map<int, TcpconnectionPtr> ConnectionMap;

private:
    EventLoop* loop_;
    int next_connection_id_;
    std::unique_ptr<EventLoopThreadPool> threads_;
    std::unique_ptr<Acceptor> acceptor_;
    const std::string ipport_;

    TcpConnectionTask connection_task_;
    TcpMessageTask    message_task_;
    ConnectionMap     connections_;

public:
    TcpServer(EventLoop* loop, const Address& address);
    ~TcpServer();

    void Start() {
        threads_->Start();
        INFO("TcpServer: threadpoll started!");
        loop_->ToRun(std::bind(&Acceptor::Listen, acceptor_.get()));
    }

    void SetConnectionTask(TcpConnectionTask&& callback) { 
        connection_task_ = std::move(callback);
    }

    void SetConnectionTask(const TcpConnectionTask& callback) { 
        connection_task_ = callback;
    }

    void SetMessageTask(TcpMessageTask&& callback) {
        message_task_ = std::move(callback);
    }

    void SetMessageTask(const TcpMessageTask& callback) {
        message_task_ = callback;
    }

    void SetThreadNums(int thread_nums) {
        threads_->SetThreadNum(thread_nums);    
    }

    inline void OnClose(const TcpConnectionPtr& conn);

    /**
     * @brief 抹除这个 connection
     * @param ptr connection 
     * 
     * @note 作为一个 task 让 loop 调用
     */
    inline void CloseTask(const TcpConnectionPtr& ptr);
    inline void OnNewConnection(int connfd, const Address& address);

};



} // namespace mew
#endif // MEW_TCPSERVER_H