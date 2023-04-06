#include <thread>
#include <atomic>

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <assert.h>

#include "CountDownLatch.h"
#include "Thread.h"


using namespace mew;

namespace mew::current_thread {


pid_t gettid() {
    return static_cast<int>(syscall(SYS_gettid));
}


} // namespace mew::current_thread;



namespace mew {

std::atomic<int> Thread::s_num_thread_created = 0;

namespace current_thread {
    thread_local std::shared_ptr<ThreadData> thread_data = nullptr;
} // namespace current_thread

using current_thread::thread_data;


void Thread::run_func(CountDownLatch &latch) {
    thread_data = data_;
    latch.CountDown();

    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "%d", (++Thread::s_num_thread_created));
    auto &name = thread_data->name_;
    prctl( PR_SET_NAME, name.size() == 0 ? ("WorkerThread " + std::string(buf)).data() : name.data());

    thread_data->tid_ = gettid();

    thread_data->func_();
}


Thread::Thread(ThreadFunc thread_func, std::string_view name)
    : data_(std::make_shared<ThreadData>(thread_func, name)),
        thread_(nullptr)
{}


void Thread::Start() {
    assert(thread_ == nullptr && "thread can only start once");
    CountDownLatch latch { 1 };
    thread_ = std::make_unique<std::thread>( &Thread::run_func, this, std::ref(latch) );
    latch.Wait();
}


void Thread::Join() {
    assert( thread_ != nullptr && "try to join a thread that is not stared yet" );
    assert( thread_->joinable() && "try to join a not joinable thread" );
    thread_->join();
}

void Thread::Detach() {
    assert( thread_ != nullptr && "try to detach a thread that is not stared yet" );
    assert( thread_->joinable() && "try to detach a not joinable thread" );
    thread_->detach();
}


ThreadData::ThreadData(ThreadFunc &func, std::string_view name)
    : name_(name), func_(std::move(func)), tid_(0)
{}


} // namespace mew


