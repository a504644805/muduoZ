#ifndef MUDUOZ_SRC_BASE_LOGGER_H
#define MUDUOZ_SRC_BASE_LOGGER_H

#include "LogStream.h"
#include "noncopyable.h"
/* Usage: LOG_INFO << ... ; */
// This file will be included many place, just expose necessary interface(like pimpl)
class Logger : muduoZ::noncopyable {
   public:
    enum LogLevel {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        SYSERR,
        SYSFATAL,
        NUM_LOG_LEVELS
    };
    Logger() = delete;
    Logger(const char* filename, int line, LogLevel level, const char* funcname);
    ~Logger();
    static void setLogLevel(LogLevel level) { logLevel = level; }
    static LogLevel getLogLevel() { return logLevel; }
    LogStream& stream() { return stream_; }

   private:
    LogStream stream_;

    static LogLevel logLevel;  // global log level
    LogLevel level_;           // current line's level;
};

#define LOG_TRACE                               \
    if (Logger::getLogLevel() <= Logger::TRACE) \
    Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()

#define LOG_DEBUG                               \
    if (Logger::getLogLevel() <= Logger::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()

#define LOG_INFO                               \
    if (Logger::getLogLevel() <= Logger::INFO) \
    Logger(__FILE__, __LINE__, Logger::INFO, __func__).stream()

#define LOG_WARN \
    Logger(__FILE__, __LINE__, Logger::WARN, __func__).stream()

#define LOG_ERROR \
    Logger(__FILE__, __LINE__, Logger::ERROR, __func__).stream()

#define LOG_SYSERR \
    Logger(__FILE__, __LINE__, Logger::SYSERR, __func__).stream()

#define LOG_SYSFATAL \
    Logger(__FILE__, __LINE__, Logger::SYSFATAL, __func__).stream()

#endif