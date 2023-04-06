#ifndef MEW_LOGAGENT_H
#define MEW_LOGAGENT_H

#include <functional>
#include <string>
#include <cstring>
#include <ctime>
#include <stdarg.h>
#include <assert.h>
#include <stdio.h>

#include "AsyncLogger.h"
#include "../base/Thread.h"

namespace mew {



class LogAgent {
public:
    using Buffer = FixedBuffer<BUF_SMALLSIZE>;

    enum LogLevel {
        TRACE = 0,  DEBUG,  INFO,
        WARN,       ERROR,  FATAL,
        NUM_LOG_LEVELS
    };
    constexpr static const char* level2str[6] = {
        "TRACE",     "DEBUG",    "INFO ",
        "WARN ",     "ERROR",    "FATAL"
    };      // 5个字母的长度，space padded
    inline static const size_t LEVEL_STR_LEN = 5;

private:
    Buffer               buf_;
    const char          *filename_;
    decltype(__LINE__)   line_;
    size_t               level_;
    
public:
    LogAgent(LogLevel level, const char* filename, int line);

    void Log(const char *format, ...);

    ~LogAgent();

    bool Flush();
};

inline static LogAgent::LogLevel s_level_filter = LogAgent::INFO;


#if 1  
    #define TRACE(msg, ...) LogAgent(LogAgent::TRACE, __FILE__, __LINE__).Log(msg, ##__VA_ARGS__)
    #define DEBUG(msg, ...) LogAgent(LogAgent::DEBUG, __FILE__, __LINE__).Log(msg, ##__VA_ARGS__)
    #define INFO(msg, ...)  LogAgent(LogAgent::INFO,  __FILE__, __LINE__).Log(msg, ##__VA_ARGS__)
    #define WARN(msg, ...)  LogAgent(LogAgent::WARN,  __FILE__, __LINE__).Log(msg, ##__VA_ARGS__)
    #define ERROR(msg, ...) LogAgent(LogAgent::ERROR, __FILE__, __LINE__).Log(msg, ##__VA_ARGS__)
    #define FATAL(msg, ...) LogAgent(LogAgent::FATAL, __FILE__, __LINE__).Log(msg, ##__VA_ARGS__)

#else   // 压测时用
    #define DEBUG(msg, ...) // LogAgent(LogAgent::DEBUG, __FILE__, __LINE__).log(msg, ##__VA_ARGS__)
    #define INFO(msg, ...)  // LogAgent(LogAgent::INFO,  __FILE__, __LINE__).log(msg, ##__VA_ARGS__)
    #define WARN(msg, ...)  // LogAgent(LogAgent::WARN,  __FILE__, __LINE__).log(msg, ##__VA_ARGS__)
    #define ERROR(msg, ...) // LogAgent(LogAgent::ERROR, __FILE__, __LINE__).log(msg, ##__VA_ARGS__)
    #define FATAL(msg, ...) // LogAgent(LogAgent::FATAL, __FILE__, __LINE__).log(msg, ##__VA_ARGS__)
#endif 

/**
 * @brief 先启动了才能打日志, 需要时可以来修改这个函数实现
*/
void LOG_INIT(const char* filename = "./webserver.log", int64_t flush_interval = 2000);


inline void LOG_SET_LEVEL( LogAgent::LogLevel lowest_level ) {
    s_level_filter = lowest_level;
}


} // namespace mew 
#endif // MEW_LOGAGENT_H