#include <assert.h>

#include "AsyncLogger.h"


namespace mew {

void AsyncLogger::copy_into_current_buf_(const char *msg, size_t n) {
    memcpy(current_buf_->Cursor(), msg, n);
    current_buf_->CursorAdvance(n);
    *current_buf_->Cursor() = '\n';
    current_buf_->CursorAdvance(1);
}


AsyncLogger::AsyncLogger(const std::string& filename, int64_t flush_interval) 
    : is_running_(false),
      flush_interval_(flush_interval),
      filename_(filename),
      current_buf_(new Buffer()),
      aux_buf_(new Buffer()),
      buffers_(),
      mtx_(),
      cond_(),
      thread_(std::bind(&AsyncLogger::MainTask, this), "logger_thread"),
      latch_(1)       // 为了能够开始
{
    buffers_.reserve(16);
}

    

// 非日志线程调用的
void AsyncLogger::Append(const char *msg, size_t n) {
    std::unique_lock ul { mtx_ };
    if (current_buf_->AvailableBytes() <= n) {
        buffers_.emplace_back(std::move(current_buf_));
        if (aux_buf_) 
            current_buf_ = std::move(aux_buf_);
        else 
            current_buf_.reset(new Buffer());
    }
    copy_into_current_buf_(msg, n);
    cond_.notify_one();
}

// 日志线程函数
void AsyncLogger::MainTask() {
    
    assert(is_running_ == true);
    latch_.CountDown();

    LogFile output(filename_);
    BufferPtr temp_buf1(new Buffer());
    BufferPtr temp_buf2(new Buffer());
    BufferPtrVec bufs_to_write;
    bufs_to_write.reserve(16);

    while (is_running_) {
        assert(bufs_to_write.empty());

        {   // 先等待 buffers_ 有内容, 等了还是没有的话, 就去找current_buf_要内容
            std::unique_lock lock { mtx_ };
            while ( buffers_.empty() ) {
                cond_.wait_for(lock, std::chrono::milliseconds(flush_interval_));
                if (  !current_buf_->empty() ) break;   // 过了一段时间后如果有东西就不等了
            }
            buffers_.emplace_back(std::move(current_buf_));

            current_buf_ = std::move(temp_buf1);

            bufs_to_write.swap(buffers_);

            if (!aux_buf_) {
                aux_buf_ = std::move(temp_buf2);    
            }
            
            assert(buffers_.empty());
        }

        assert(!bufs_to_write.empty());


        if (bufs_to_write.size() > 25) {
            char buf[256];
            snprintf(buf, sizeof(buf), "Dropped log messages at %s, %zd larger buffers\n",
                Timestamp::now().to_string().c_str(),
                bufs_to_write.size()-2
            );
            fputs(buf, stderr);
            output.Append(buf, static_cast<int>(strlen(buf)));
            bufs_to_write.erase(bufs_to_write.begin()+2, bufs_to_write.end());
        }

        // 写数据
        for (const BufferPtr &buf : bufs_to_write) {
            output.Append(buf->begin(), buf->length());
        }

        // 尽量利用 bufs_to_write中没用的内存 更新 temp_buf, 减少 申请内存
        if (temp_buf1 == nullptr) {
            if (bufs_to_write.empty()) {
                temp_buf1.reset(new Buffer());
            }
            else {
                temp_buf1 = std::move(bufs_to_write.back());
                bufs_to_write.pop_back();
                temp_buf1->clear();
            }
        }

        if (temp_buf2 == nullptr) {
            if (bufs_to_write.empty()) {
                temp_buf2.reset(new Buffer());
            }
            else {
                temp_buf2 = std::move(bufs_to_write.back());
                bufs_to_write.pop_back();
                temp_buf2->clear();
            }
        }

        bufs_to_write.clear();
        output.Flush();
    }
    output.Flush();
}

// 用户要确保只有一个线程调用
void AsyncLogger::Start() {
    is_running_ = true;
    thread_.Start(); 
    latch_.Wait();
}

void AsyncLogger::Join() {
    is_running_ = false;
    thread_.Join();
    
}


} // namespace mew