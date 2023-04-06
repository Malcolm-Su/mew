
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>

#include "../utils/Buffer.h"
#include "../log/Log.h"
#include "TcpConnection.h"
#include "Event.h"
#include "EventLoop.h"

#include "Log.h"

using namespace mew;



void TcpConnection::SetupCloseTimer() {
    auto timeout_task = [](std::weak_ptr<TcpConnection> wp){
        std::shared_ptr<TcpConnection> sp = wp.lock();
        if (!sp) return;
        sp->OnClose();
    };
    loop_->ToRunAfter( close_timeout_, std::bind(timeout_task, weak_from_this() ) );
}


TcpConnection::TcpConnection(EventLoop* loop, int connfd, int id) 
    : loop_(loop),
      connection_id_(id),
      state_(State::kConnecting),
      event_(new Event(loop_, connfd))
{
    event_->SetReadTask( std::bind(&TcpConnection::OnMessage, this) );
    event_->SetWriteTask( std::bind(&TcpConnection::OnWrite, this) );
    event_->SetErrorTask( std::bind(&TcpConnection::OnError, this) );
}

TcpConnection::~TcpConnection() {
    // event_->AbandonAndUpdate(); 
}


void TcpConnection::OnConnection() {
    state_ = State::kConnected;
    event_->SetTie( shared_from_this() );
    event_->EnableAndUpdate(Event::kRead);
    connection_task_(shared_from_this(), &input_buffer_);
}


int TcpConnection::GetErrno() const {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(event_->GetFd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}



void TcpConnection::OnClose() {
    state_ = State::kDisconnected;
    event_->AbandonAndUpdate();
    TcpConnectionPtr guard(shared_from_this());
    close_task_(guard);
    // this should be near the end of the object, as well as the socket fd.
}


void TcpConnection::OnError() {
    ERROR( "TcpConnection::OnError:");
}


void TcpConnection::OnMessage() {
    int read_size = input_buffer_.ReadFd(event_->GetFd());
    if (read_size > 0) {
        message_task_(shared_from_this(), &input_buffer_);
    } 
    else if (read_size == 0) {    // 需要关闭的时候
        if (state_ == State::kDisconnecting) {  // 主动关闭等待对端关闭的时候
            OnClose();
        }
        else {      // 被动关闭的时候
            state_ = State::kDisconnecting;
            SetupCloseTimer();
            if (!event_->IsWriteEnabled()) {  // 不能写的话，直接关闭了
                OnClose();
            }
        }
    } 
    else {
        ERROR( "TcpConnection::OnMessage failed" );
    }
    
}


int TcpConnection::FlushOutputBuffer() {
    int remain_size = output_buffer_.readablebytes();
    if (remain_size <= 0) return 0;
    
    int sent_size = static_cast<int>(::write(event_->GetFd(), output_buffer_.Peek(), remain_size));
    output_buffer_.Retrieve(sent_size);

    return sent_size;
}


void TcpConnection::OnWrite() {
    int sent_size = FlushOutputBuffer();
    if (sent_size < 0) {
        if (errno == EPIPE) {      // 对端 close 了
            OnClose();
        } else if (errno != EWOULDBLOCK) {
            // tcp send buffer 满的时候 errno == EWOULDBLOCK
            // 此时不给予处理，等待下次可读再写，水平触发方便保证
            ERROR( "TcpConnection::OnWrite failed for %s", strerror(errno));
        }
        return;
    }

    if (output_buffer_.readablebytes() > 0) 
        return;
    
    event_->DisableAndUpdate(Event::kWrite);
    if ( IsShutdown() ) {
        RealShutdown();
        state_ = State::kDisconnecting;
    } else if (state_ == State::kDisconnecting) {
        OnClose();
    }
}


// send in the same thread
void TcpConnection::Send(const char* message, int len) {
    int remaining = len;
    int send_size = 0;

    // try to send directly if there is nothing in output buffer
    if (!event_->IsWriteEnabled() && output_buffer_.readablebytes() == 0) {
        send_size = static_cast<int>(::write(event_->GetFd(), message, len));
        if (send_size >= 0) {
            remaining -= send_size; 
        } else {
            if (errno != EWOULDBLOCK) {
                ERROR( "TcpConnection::Send write failed" );
            } 
            return;
        }
    }

    assert(remaining <= len);
    if (remaining > 0) {
        output_buffer_.Append(message + send_size, remaining);
        if (!event_->IsWriteEnabled()) {
            event_->EnableAndUpdate(Event::kWrite);
        }
    }
}

void TcpConnection::Shutdown() { 
    assert(state_ != State::kDisconnecting);
    if ( IsShutdown() ) return;
    shutdown_flag_ = true;

    SetupCloseTimer();
    state_ = State::kDisconnecting;
    if ( !event_->IsWriteEnabled() ) {
        RealShutdown();
    }
}


void TcpConnection::RealShutdown()  {
    assert(shutdown_flag_);
    int ret = ::shutdown(event_->GetFd(), SHUT_WR);
    if (ret < 0) {
        ERROR( "TcpConnection::shutdown failed for %s, fd:%d", strerror(errno), event_->GetFd() );
    }
    if (!event_->IsReadEnabled() || ret < 0) {
        OnClose();
    }
}


// send in the same thread
void TcpConnection::Send(Buffer* buffer) {
    if (state_ == State::kDisconnected) return; 
    Send(buffer->Peek(), buffer->readablebytes()); 
    buffer->RetrieveAll();
}

// send in the same thread
void TcpConnection::Send(const string& message) {
    if (state_ == State::kDisconnected) return; 
    Send(message.data(), static_cast<int>(message.size()));
}
