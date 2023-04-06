#ifndef MEW_THREAD_H
#define MEW_THREAD_H

#include <functional>
#include <string>
#include <thread>

#include <pthread.h>
#include <sys/prctl.h>

#include "CountDownLatch.h"
#include "NonCopyable.h"

namespace mew {

struct ThreadData;

class Thread : public NonCopyable {
public:
    using ThreadFunc = std::function<void()>;
    static std::atomic<int> s_num_thread_created;

private:
    std::shared_ptr<ThreadData> data_;
    std::unique_ptr<std::thread> thread_;

private:
    void run_func(CountDownLatch &latch);

public:
    Thread(ThreadFunc thread_func, std::string_view name = "");
    ~Thread() = default;

public:
    void Start();
    void Join();
    void Detach();

};  // class Thread


struct ThreadData {
    using ThreadFunc = std::function<void()>;

    std::string name_;
    ThreadFunc  func_;
    pid_t       tid_;

    ThreadData(ThreadFunc &func, std::string_view name);
};  // struct ThreadData



} // namespace mew


namespace mew::current_thread {
extern thread_local std::shared_ptr<ThreadData> thread_data;    

inline const std::string& GetName() { return thread_data->name_; }
inline pid_t GetTid() { return thread_data->tid_; }

inline void SetupInfo(std::function<void()> func, std::string_view name ) {
    thread_data.reset( new ThreadData( func, name ) );
}


}   // namespace mew::current_thread


#endif // MEW_THREAD_H