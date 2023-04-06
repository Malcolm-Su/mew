#ifndef MEW_HTTPPARSESTATE_H
#define MEW_HTTPPARSESTATE_H

namespace mew {

enum HttpRequestParseState {
    kParseRequestLine,
    kParseHeaders,
    kParseBody,
    kParseGotCompleteRequest,
    kParseErrno,
};

} // namespace mew

#endif // MEW_HTTPPARSESTATE_H
