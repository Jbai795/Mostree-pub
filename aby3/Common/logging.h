#pragma once
#ifndef CW_CORE_LOGGING_H_
#define CW_CORE_LOGGING_H_

#include <atomic>
#include <limits>
#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>

namespace CW {
    const int INFO = 0;
    const int WARNING = 1;
    const int ERROR = 2;
    const int FATAL = 3;
    const int NUM_SEVERITIES = 4;

    namespace LOGINTER {

#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */

        class LogMessage : public std::basic_ostringstream<char> {
        public:
            LogMessage(const char *fname, int line, int severity);

            ~LogMessage() override;

            // Change the location of the log message.
            LogMessage &AtLocation(const char *fname, int line);

        protected:
            void GenerateLogMessage();

        private:
            const char *fname_;
            int line_;
            int severity_;
            //std::mutex log_lock;
        };

// LogMessageFatal ensures the process will exit in failure after
// logging this message.
        class LogMessageFatal : public LogMessage {
        public:
            LogMessageFatal(const char *file, int line);

            ~LogMessageFatal() override;
        };

    }  // namespace internal

#define _CW_LOG_INFO \
  ::CW::LOGINTER::LogMessage(__FUNCTION__, __LINE__, ::CW::INFO)
#define _CW_LOG_WARNING \
  ::CW::internal::LogMessage(__FUNCTION__, __LINE__, ::CW::WARNING)
#define _CW_LOG_ERROR \
  ::CW::LOGINTER::LogMessage(__FUNCTION__, __LINE__, ::CW::ERROR)
#define _CW_LOG_FATAL ::CW::internal::LogMessageFatal(__FUNCTION__, __LINE__)

#define _CW_LOG_QFATAL _CW_LOG_FATAL

#define LOG(severity) _CW_LOG_##severity
}  // namespace CW

#endif  // PEGASUS_CORE_LOGGING_H_
