#ifndef __PISTIS__LOGGING__HELPERS__TESTINGLOG_HPP__
#define __PISTIS__LOGGING__HELPERS__TESTINGLOG_HPP__

#include <pistis/logging/Log.hpp>

namespace pistis {
  namespace logging {

    class TestingLog : public Log {
    public:
      TestingLog(LogMessageFactory* msgFactory,
		 LogMessageReceiver* msgReceiver,
		 const std::string& destination,
		 LogLevel logLevel);

      void setLogLevel(LogLevel l) { setLogLevel_(l); }
    };

  }
}
#endif

