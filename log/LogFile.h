
#ifndef LOGFILE_H
#define LOGFILE_H

#include <memory>
#include <string>
#include <mutex>
#include "FileUtil.h"

namespace mew {

// TODO 提供自动归档功能
class LogFile {
private:
    const std::string basename_;
    const int flushEveryN_;
    int count_;
    std::unique_ptr<std::mutex> mutex_;
    std::unique_ptr<AppendFile> file_;

private:
    void append_unlocked_(const char* logline, int len);

    LogFile(const LogFile&) = delete;
    LogFile& operator=(const LogFile&) = delete;

public:
    LogFile(const std::string& basename, int flushEveryN = 1024);
    ~LogFile();

public:
    void Append(const char* logline, int len);
    void Flush();
    bool RollFile();    // TODO
};

} // namespace mew 
#endif // LOGFILE_H
