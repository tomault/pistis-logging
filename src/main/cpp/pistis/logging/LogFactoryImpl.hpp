#ifndef __PISTIS__LOGGING__LOGFACTORYIMPL_HPP__
#define __PISTIS__LOGGING__LOGFACTORYIMPL_HPP__

#include <pistis/logging/Log.hpp>

namespace pistis {
  namespace logging {

    class LogFactoryImpl {
    public:
      virtual ~LogFactoryImpl() { }
      virtual Log* getLog(const std::string& destination) = 0;
      virtual void setLogLevel(const std::string& destination,
			       LogLevel logLevel) = 0;
    };

  }
}
#endif

