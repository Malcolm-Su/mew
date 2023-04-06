
#include <utility>
#include <functional>

#include <assert.h>
#include <climits>

#include "Address.h"
#include "TcpServer.h"
#include "../base/Callback.h"


using namespace mew;

TcpServer::TcpServer(EventLoop* loop, const Address& address)
    : loop_(loop),
      next_connection_id_(1),
      threads_(new EventLoopThreadPool(loop_)),
      acceptor_(new Acceptor(loop_, address)),
      ipport_(address.ToString())
{
    acceptor_->SetNewConnectionTask(std::bind(&TcpServer::OnNewConnection, this, _1, _2));
    INFO("TcpServer initialized!");
}


TcpServer::~TcpServer() {
    for (auto& pair : connections_) {
        TcpConnectionPtr ptr(pair.second);
        pair.second.reset();
        ptr->GetLoop()->ToRun(std::bind(&TcpConnection::OnClose, ptr));
    }
}


inline void TcpServer::OnClose(const TcpConnectionPtr& ptr) {
    loop_->ToRun(std::bind(&TcpServer::CloseTask, this, ptr));
}


inline void TcpServer::CloseTask(const TcpConnectionPtr& ptr) {
    assert(connections_.find(ptr->GetFd()) != connections_.end());

    INFO( "TcpServer::CloseTask - remove connection [%s#%u]", 
        ipport_.c_str(), ptr->GetId()
    );

    connections_.erase( connections_.find(ptr->GetFd()) );
}



inline void TcpServer::OnNewConnection(int connfd, const Address& address) {
    EventLoop* loop = threads_->GetNextLoop(); 
    TcpConnectionPtr ptr(new TcpConnection(loop, connfd, next_connection_id_));
    connections_[connfd] = ptr;

    ptr->SetConnectionTask(connection_task_);
    ptr->SetMessageTask(message_task_);
    ptr->SetCloseTask(std::bind(&TcpServer::OnClose, this, _1));

    INFO( "TcpServer::OnNewConnection - new connection [%s#%d] from %s", 
        ipport_.c_str(), next_connection_id_,  address.ToString().c_str()
    );

    ++next_connection_id_;
    if (next_connection_id_ == INT_MAX) next_connection_id_ = 1;
    loop->ToRun(std::bind(&TcpConnection::OnConnection, ptr));
}
