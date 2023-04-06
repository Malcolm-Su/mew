#ifndef MEW_HTTPREQUEST_H
#define MEW_HTTPREQUEST_H


#include <string>
#include <map>

namespace mew {


static const char http[] = "HTTP/1.";

enum Method {
    kGet,
    kPost,    // to be implemented...      
    kPut,     // to be implemented...      
    kDelete,  // to be implemented...      
    kTrace,   // to be implemented...  
    kOptions, // to be implemented...      
    kConnect, // to be implemented...      
    kPatch    // to be implemented...  
};

enum Version {
    kUnknown,
    kHttp10,
    kHttp11
};

class HttpRequest {
private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    std::map<std::string, std::string> headers_;

public:
    HttpRequest();

    ~HttpRequest();

public:

    bool ParseRequestMethod(const char* start, const char* end);

    bool ParseRequestLine(const char* start, const char* end);

    bool ParseHeaders(const char* start, const char* colon, const char* end);

    bool ParseBody(const char* start, const char* end);

    Method method() const { return method_; }
    const std::string& path() const { return path_; }
    const std::string& query() const { return query_; }
    Version version() const { return version_; }
    const std::map<std::string, std::string>& headers() const { return headers_; }

    void Swap(HttpRequest& req);

    std::string GetHeader(const std::string& header) const {
        std::string ret;
        auto iter = headers_.find(header);
        return iter == headers_.end() ? ret : iter->second;
    }

};


} // namespace mew
#endif // MEW_HTTPREQUEST_H