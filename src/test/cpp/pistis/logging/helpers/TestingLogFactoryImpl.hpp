#ifndef __PISTIS__LOGGING__HELPERS__TESTINGLOGFACTORYIMPL_HPP__
#define __PISTIS__LOGGING__HELPERS__TESTINGLOGFACTORYIMPL_HPP__

#include <pistis/logging/LogFactoryImpl.hpp>
#include <map>
#include <memory>

#include "TestingLog.hpp"
#include "TestingLogMessageReceiver.hpp"
#include "TrackingLogMessageFactory.hpp"

namespace pistis {
  namespace logging {

    class TestingLogFactoryImpl : public LogFactoryImpl {
    public:
      TestingLogFactoryImpl();
      virtual ~TestingLogFactoryImpl();

      size_t numGetLogCalls() const { return numGetLogCalls_; }
      size_t numSetLogLevelCalls() const { return numSetLogLevelCalls_; }

      virtual TestingLog* getLog(const std::string& destination);
      virtual void setLogLevel(const std::string& destination,
			       LogLevel logLevel);

      static size_t initialMessageSize() { return 256; }
      static size_t maxMessageSize() { return 65536; }
      static LogLevel defaultLogLevel() { return LogLevel::TRACE; }
    private:
      typedef std::map<std::string, TestingLog*> LogMapType;

      LogMapType logs_;
      std::unique_ptr<TrackingLogMessageFactory> msgFactory_;
      std::unique_ptr<TestingLogMessageReceiver> msgReceiver_;
      size_t numGetLogCalls_;
      size_t numSetLogLevelCalls_;
    };

    LogFactoryImpl* createLogFactoryImpl();

  }
}
#endif

