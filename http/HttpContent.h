#ifndef MEW_HTTPCONTENT_H
#define MEW_HTTPCONTENT_H

#include <utility>
#include <algorithm>


#include "../utils/Buffer.h"
#include "HttpParseState.h"
#include "HttpRequest.h"


namespace mew {

class HttpContent {
private:
    HttpRequest request_;
    HttpRequestParseState parse_state_;
    
public:
    HttpContent();
    ~HttpContent();

public:
    bool ParseContent(Buffer* buffer);
    bool GetCompleteRequest() { return parse_state_ == kParseGotCompleteRequest; } 
    
    const HttpRequest& request() const { return request_; }
    void ResetContentState() { 
        HttpRequest tmp;
        request_.Swap(tmp);
        parse_state_ = kParseRequestLine;
    }

};



} // namespace mew
#endif // MEW_HTTPCONTENT_H