#include "Logger.h"

#include "Timestamp.h"
Logger::LogLevel Logger::logLevel = Logger::TRACE;

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
    {
        "TRACE ",
        "DEBUG ",
        "INFO  ",
        "WARN  ",
        "ERROR ",
        "SYSERR ",
        "SYSFATAL ",
};
Logger::Logger(const char* filename, int line, LogLevel level, const char* funcname) : level_(level) {
    stream_ << LogLevelName[level] << filename << " in function " << funcname << " Line " << line
            << " at " << Timestamp::now().toFormattedString().c_str();
    if (level_ >= ERROR) {
        stream_ << " errno = " << errno;
    }
    stream_ << ": ";
}

extern AsyncLogging g_asynclogging;
Logger::~Logger() {
    if (level_ == ERROR || level_ == SYSFATAL) {
        fprintf(stderr, "%s", stream_.getBuffer().begin());
        abort();
    } else {
        g_asynclogging.append(stream_.getBuffer().begin(), stream_.getBuffer().len());
        g_asynclogging.append("\n", 1);
        if (stream_.getOverLen())
            g_asynclogging.append(" This msg too long, some data is discarded ", 43);  // New thing: when LogStream's buffer is not large enough, append some information to notify.
    }
}