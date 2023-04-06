#ifndef MEW_HTTPSERVER_H
#define MEW_HTTPSERVER_H

#include <memory>
#include <functional>
#include <utility>

#include "../net/EventLoop.h"
#include "../net/Address.h"
#include "../net/TcpConnection.h"
#include "../net/TcpServer.h"
#include "../base/Callback.h"
#include "../log/Log.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "../time/Time.h"

namespace mew {

static constexpr time::seconds IDLE_TIMEOUT = 8s;

class HttpServer {
public:
    using HttpResponseTask = std::function<void(const HttpRequest&, HttpResponse&)>;

private:
    EventLoop* loop_;
    TcpServer  server_;
    bool       auto_close_flag_;

    HttpResponseTask response_task_;
    
public:
    HttpServer(EventLoop* loop, 
               const Address& address, 
               bool is_auto_close = false
              );

    ~HttpServer() = default;

public:
    void Start() { server_.Start(); }

    /**
     * @brief 默认的处理方式，就是404
     * @param request 
     * @param response 
     */
    void DefaultResponseTask(const HttpRequest& request, HttpResponse& response) {
        response.SetStatusCode(k404NotFound);
        response.SetStatusMessage("Not Found");
        response.SetCloseConnection(true);
    }


    /**
     * @brief 超时处理, 将超时的连接关闭 / lazily update timeout if needed, 
     * @param connection 
     */
    void OnIdle(std::weak_ptr<TcpConnection>& connection) {
        TcpConnectionPtr conn(connection.lock());
        if (!conn) return;  // 连接已经没了的时候就算了

        // 超时的时候就关闭连接
        if ( Timestamp::from( conn->GetLastActiveTime(), IDLE_TIMEOUT ) < Timestamp::now() ) {
            conn->Shutdown();
        } 
        else {  // 如果有该连接有更新过的话, 上面的if就会false, 就落在这里, 
                // 说明有活动过,需要重新设置timeout, 这是一种lazy的timeout update手段
            loop_->ToRunAfter( IDLE_TIMEOUT, std::move(std::bind(&HttpServer::OnIdle, this, connection))); 
        }
    }

    void OnConnection(const TcpConnectionPtr& connection) {
        connection->SetContext( std::make_shared<HttpContent>() );
        if ( !auto_close_flag_ ) return;
        loop_->ToRunAfter( IDLE_TIMEOUT, std::bind(&HttpServer::OnIdle, this, std::weak_ptr<TcpConnection>(connection)) ); 
    }
    
    /**
     * @brief 处理收到的信息
     * @param connection 
     * @param buffer 
     */
    void OnMessage(const TcpConnectionPtr& connection, Buffer* buffer);

    void SetResponseTask(const HttpResponseTask& response_callback) { 
        response_task_ = response_callback; 
    }

    void SetResponseTask(HttpResponseTask&& response_callback) { 
        response_task_ = std::move(response_callback); 
    } 
    
    void SetThreadnums(int thread_nums) {
        server_.SetThreadNums(thread_nums);
    }

    void OnRequest(const HttpRequest& request, const TcpConnectionPtr& connection);

};

} // namespace mew
#endif // MEW_HTTPSERVER_H