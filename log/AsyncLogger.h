#ifndef MEW_ASYNCLOGGER_H
#define MEW_ASYNCLOGGER_H

#include <memory>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <time.h>
#include <string.h>


#include "FixedBuffer.h"
#include "../base/Thread.h"
#include "../base/CountDownLatch.h"
#include "../time/Time.h"
#include "LogFile.h"

using mew::time::Timestamp;

namespace mew {


class AsyncLogger {
public:
    using Buffer = FixedBuffer<BUF_LARGESIZE>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferPtrVec = std::vector<BufferPtr>;
    
private:
    bool                    is_running_;
    int64_t                 flush_interval_;        // in ms
    std::string             filename_;
    BufferPtr               current_buf_;
    BufferPtr               aux_buf_;
    BufferPtrVec            buffers_;
    std::mutex              mtx_;
    std::condition_variable cond_;
    Thread                  thread_;
    CountDownLatch          latch_;
    

    void copy_into_current_buf_(const char *msg, size_t n);

public:
    /**
     * @brief 创建日志线程, 这个线程才是写 log 的真正线程
     * @param filename 日志的文件路径
     * @param flush_interval 定时写的间隔, 毫秒为单位
    */
    AsyncLogger(const std::string& filename, int64_t flush_interval);

    // 非日志线程调用的
    void Append(const char *msg, size_t n);

    // 日志线程函数
    void MainTask();

    // 用户要确保只有一个线程调用
    void Start();

    void Join();

};  // class AsyncLogger
} // namespace mew



#endif // MEW_ASYNCLOGGER_H