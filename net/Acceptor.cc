#include <functional>

#include <assert.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <bits/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>


#include "Acceptor.h"
#include "Address.h"
#include "Event.h"
#include "../log/Log.h"

using namespace mew;

Acceptor::Acceptor(EventLoop* loop, const Address& address)
    : loop_(loop),
      listenfd_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)),
      idlefd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
      event_(new Event(loop_, listenfd_)) 
{
    SetSockOptReuseaddr(listenfd_);
    SetSockeOptKeepAlive(listenfd_);
    Bind(address);
    event_->SetReadTask(std::bind(&Acceptor::OnNewConnection, this));
}

Acceptor::~Acceptor() {
    event_->AbandonAndUpdate();
    ::close(listenfd_);
}

void Acceptor::Bind(const Address& addr) {
    struct sockaddr_in address;
    bzero((char*)&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY); 
    address.sin_port = htons(static_cast<uint16_t>(addr.GetPort()));
    int ret = bind(listenfd_, (struct sockaddr*)(&address), sizeof(address));
    assert(ret != -1); 
}

void Acceptor::Listen() {
    int ret = ::listen(listenfd_, SOMAXCONN);
    assert(ret != -1);
    event_->EnableAndUpdate( Event::kRead );
}

void Acceptor::OnNewConnection() {
    struct sockaddr_in client, peeraddr;
    socklen_t client_addrlength = sizeof(client);
    int connfd = ::accept4(listenfd_, (struct sockaddr*)&client, &client_addrlength, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0) {
        if (errno == EMFILE) {
            ::close(idlefd_);
            idlefd_ = ::accept(listenfd_, NULL, NULL);
            ::close(idlefd_);
            idlefd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
        return;
    }
    assert(connfd > 0);
    if (SetSockeOptKeepAlive(connfd) == -1) {
        ERROR("Acceptor::NewConnection SetSockoptKeepAlive failed");
        close(connfd);
        return;
    }
    
    if (SetSockOptTcpNoDelay(connfd) == -1) {
        ERROR("Acceptor::NewConnection SetSockoptTcpNoDelay failed");
        close(connfd);
        return;
    }

    socklen_t peer_addrlength = sizeof(peeraddr);
    getpeername(connfd, (struct sockaddr *)&peeraddr, &peer_addrlength);
    new_connection_task_(connfd, Address(::inet_ntoa(peeraddr.sin_addr), ::ntohs(peeraddr.sin_port)));
}
