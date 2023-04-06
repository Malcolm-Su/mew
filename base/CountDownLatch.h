#ifndef MEW_DOUNTDOWNLATCH_H
#define MEW_DOUNTDOWNLATCH_H

#include <mutex>
#include <condition_variable>

#include "NonCopyable.h"

namespace mew {


class CountDownLatch : public NonCopyable {
private:
    std::mutex              mtx_;
    std::condition_variable cond_;
    int count_;

public:
    explicit CountDownLatch(int count)
        : mtx_(),
          cond_(),
          count_(count)
    {}

    void CountDown() {
        std::unique_lock lock { mtx_ };
        --count_;
        if (count_ == 0) {
            cond_.notify_all();
        }  
    }

    void Wait() {
        std::unique_lock lock { mtx_ };
        while (count_ > 0) {
            cond_.wait( lock );
        }
    }

    int GetCount() const { return count_; }
};



} // namespace mew
#endif // MEW_DOUNTDOWNLATCH_H