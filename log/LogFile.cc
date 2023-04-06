#include "LogFile.h"

/////////////////// implementations /////////////////////////
namespace mew {
    
LogFile::LogFile(const std::string& basename, int flushEveryN)
    : basename_(basename),
      flushEveryN_(flushEveryN),
      count_(0),
      mutex_(new std::mutex) 
{
    // assert(basename.find('/') >= 0);
    file_.reset(new AppendFile(basename));
}

LogFile::~LogFile() {}

void LogFile::Append(const char* logline, int len) {
    std::unique_lock lock(*mutex_);
    append_unlocked_(logline, len);
}

void LogFile::Flush() {
    std::unique_lock lock(*mutex_);
    file_->Flush();
}

void LogFile::append_unlocked_(const char* logline, int len) {
    file_->Append(logline, len);
    ++count_;
    if (count_ >= flushEveryN_) {
        count_ = 0;
        file_->Flush();
    }
}

} // namespace mew 