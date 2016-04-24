#ifndef __PISTIS__LOGGING__HELPERS__LOGMESSAGECONSUMER_HPP__
#define __PISTIS__LOGGING__HELPERS__LOGMESSAGECONSUMER_HPP__

#include "LockingGate.hpp"
#include <pistis/logging/LogMessageFactory.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

namespace pistis {
  namespace logging {

    class LogMessageConsumer {
    public:
      LogMessageConsumer(const std::string& name, LogMessageFactory* factory,
			 LockingGate* getGate, LockingGate* releaseGate,
			 LockingGate* waitGate,
			 const std::chrono::system_clock::time_point& deadline);
      LogMessageConsumer(const LogMessageConsumer&)= delete;
      LogMessageConsumer(LogMessageConsumer&)= delete;
      ~LogMessageConsumer();

      const std::string& name() const { return name_; }
      LogMessage* message() const { return message_; }
      const std::string& errors() const { return errors_; }
      LogMessageConsumer& operator=(const LogMessageConsumer&)= delete;

    private:
      std::string name_;
      LogMessageFactory* factory_;
      LogMessage* volatile message_;
      LockingGate* getGate_;
      LockingGate* releaseGate_;
      LockingGate* exitGate_;
      std::chrono::system_clock::time_point deadline_;
      std::string errors_;
      std::unique_ptr<std::thread> thread_;

      void run_();
    };

  }
}
#endif

