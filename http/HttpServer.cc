
#include <functional>


#include "HttpServer.h"
#include "HttpContent.h"


using namespace mew;

using mew::Version;

HttpServer::HttpServer(EventLoop* loop, const Address& address, bool is_auto_close) 
    : loop_(loop),
      server_(loop, address),
      auto_close_flag_(is_auto_close) 
{
    server_.SetConnectionTask(
        std::bind(&HttpServer::OnConnection, this, _1)
    );

    server_.SetMessageTask(
        std::bind(&HttpServer::OnMessage, this, _1, _2)
    );

    SetResponseTask(
        std::bind(&HttpServer::DefaultResponseTask, this, _1, _2)
    );
    
    INFO( "Httpserver listening on  %s:%d ", address.GetIp(), address.GetPort() );
}

// HttpServer::~HttpServer() = default;     // TODO: make it default? (checked)

void HttpServer::OnMessage(const TcpConnectionPtr& connection, Buffer* buffer) {
    
    // std::shared_ptr<HttpContent> content = static_cast<std::> connection->GetContext();
    // HttpContent *content = std::any_cast<HttpContent>(connection->GetContext());
    std::shared_ptr<HttpContent> content = connection->GetContext<HttpContent>();


    // auto-close requires updating idle time when it make a sound
    if ( auto_close_flag_ ) connection->UpdateLastActiveTime( Timestamp::now() );
    if ( connection->IsShutdown() ) return;  

    if ( !content->ParseContent(buffer) ) {
        connection->Send( "HTTP/1.1 400 Bad Request\r\n\r\n" );
        connection->Shutdown();
    }   

    if (content->GetCompleteRequest()) {
        OnRequest( content->request(), connection );
        content->ResetContentState();
    }   
}

void HttpServer::OnRequest(const HttpRequest& request, const TcpConnectionPtr& connection) {
    string connection_state = std::move( request.GetHeader("Connection") );
    bool close = (connection_state == "Close" || (request.version() == kHttp10 && connection_state != "Keep-Alive"));

    HttpResponse response( close ); 
    response_task_( request, response );
    Buffer buffer;
    response.AppendToBuffer( &buffer );
    connection->Send( &buffer );

    if ( response.CloseConnection() ) {
        connection->Shutdown();
    }   
}
