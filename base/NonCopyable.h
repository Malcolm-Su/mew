#ifndef MEW_NONCOPYABLE_H
#define MEW_NONCOPYABLE_H

namespace mew {

class NonCopyable {
protected:
    NonCopyable() {}
    ~NonCopyable() {}
 
private:
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
}; // class NonCopyable

} // namespace mew
#endif // MENONCOPYABLE_H