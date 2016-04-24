#ifndef __PISTIS__LOGGING__LOGLEVEL_HPP__
#define __PISTIS__LOGGING__LOGLEVEL_HPP__

#include <ostream>
#include <string>
#include <utility>
#include <stdint.h>

namespace pistis {
  namespace logging {

    enum class LogLevel : uint32_t {
      TRACE = 1,  ///< Extreme level of detail about the application's execution
      DEBUG = 2,  ///< Information useful for debugging the application
      INFO = 3,   ///< Informative messages about the application's behavior
      WARN = 4,   ///< Warnings, for dangerous conditions
      ERROR = 5   ///< Highest logging level; errors only
    };

    std::pair<bool, LogLevel> parseLogLevel(const std::string& text);

    const std::string& toString(LogLevel level);

    inline std::ostream& operator<<(std::ostream& out, LogLevel level) {
      return out << toString(level);
    }
  }
}
#endif
