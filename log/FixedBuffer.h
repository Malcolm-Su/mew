#ifndef MEW_FIXEDBUFFER_H
#define MEW_FIXEDBUFFER_H

#include <cstdlib>
namespace mew {
    
static const int BUF_SMALLSIZE = 4000;
static const int BUF_LARGESIZE = 4000*1000;

template <size_t MAXSIZE>
class FixedBuffer {
private:
    char buf_[MAXSIZE] = {0};
    char *cur_ = buf_;
public:
    size_t AvailableBytes() {
        return (buf_+MAXSIZE)-cur_;
    }

    char* Cursor() {
        return cur_;
    }

    char* CursorAdvance(size_t steps) {
        cur_ += steps;
        return cur_;
    }

    char* begin() {
        return buf_;
    }

    char* end() {
        return buf_ + MAXSIZE;
    }

    char* clear() {
        memset(buf_, 0, MAXSIZE);
        cur_ = buf_;
        return cur_;
    }

    size_t length() {
        return Cursor() - begin();
    }

    bool empty() {
        return Cursor() == begin();
    }

};

} // namespace mew 
#endif // MEW_FIXEDBUFFER_H