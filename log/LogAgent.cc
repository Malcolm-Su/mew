#include "LogAgent.h"

namespace mew {

static AsyncLogger *logger = nullptr;

LogAgent::LogAgent(LogLevel level, const char* filename, int line)
    : filename_(filename), line_(line), level_(level)
{}


void LogAgent::Log(const char *format, ...) {
    time_t ticks = ::time(NULL);
    struct tm *ptm = localtime(&ticks);

    char timestamp[32];
    // size_t timestamp_len = 
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", ptm);

    const char *fmt = "%s %s %d %s %s:%d ";
    auto p0 = buf_.Cursor();

    int size = snprintf(
        buf_.Cursor(), 
        buf_.AvailableBytes(), 
        fmt, 
        level2str[level_], 
        timestamp, 
        current_thread::GetTid(),
        current_thread::GetName().c_str(),
        filename_, 
        line_
    );
    buf_.CursorAdvance(size);

    va_list arg_ptr;
    va_start(arg_ptr, format);
    size = vsnprintf(buf_.Cursor(), buf_.AvailableBytes(), format, arg_ptr);
    printf("%s\n", p0);
    va_end(arg_ptr);

    buf_.CursorAdvance(size);


}

LogAgent::~LogAgent() {
    if ( s_level_filter <= level_ )
        Flush();
}


bool LogAgent::Flush() {
    // // TODO 
    // printf("buffer flush!!!\n");
    // printf("buffer content: %s\n\n", buf_.begin());

    if (logger == nullptr) return false;
    logger->Append(buf_.begin(), buf_.length());
    return true;
}


void LOG_INIT(const char* filename, int64_t flush_interval) {
    logger = new AsyncLogger(filename, flush_interval);
    logger->Start();
}


} // namespace mew 