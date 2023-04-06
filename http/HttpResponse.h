#ifndef MEW_HTTPRESPONSE_h
#define MEW_HTTPRESPONSE_h

#include <string>


namespace mew {

inline static const std::string CRLF = "\r\n";

enum HttpStatusCode {
    k100Continue = 100,
    k200OK = 200,
    k400BadRequest = 400,
    k403Forbidden = 403,
    k404NotFound = 404,
    k500InternalServerErrno = 500
};

class Buffer;



class HttpResponse {
public:
    static const std::string server_name_;

private:
    HttpStatusCode status_code_;
    std::string status_message_;
    std::string body_;
    std::string type_;
    bool close_connection_;

public:
    HttpResponse(bool close_connection) 
        : type_("text/plain"),
          close_connection_(close_connection) 
    {}

    ~HttpResponse() = default;
    
    void SetStatusCode(HttpStatusCode status_code) { status_code_ = status_code; }
    void SetStatusMessage(const std::string& status_message) { status_message_ = std::move(status_message); }
    void SetCloseConnection(bool close_connection) { close_connection_ = close_connection; }

    void SetBodyType(const std::string& type) { type_ = type; }
    void SetBody(const std::string& body) { body_ = body; }
    void AppendToBuffer(Buffer* buffer);

    bool CloseConnection() { return close_connection_; }
};



} // namespace mew
#endif // MEW_HTTPRESPONSE_h