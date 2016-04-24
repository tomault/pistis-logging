#ifndef __PISTIS__LOGGING__HELPERS__LOGMESSAGEWAITER_HPP__
#define __PISTIS__LOGGING__HELPERS__LOGMESSAGEWAITER_HPP__

#include <pistis/logging/LogMessageFactory.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "LockingGate.hpp"

namespace pistis {
  namespace logging {

    class LogMessageWaiter {
    public:
      LogMessageWaiter(const std::string& name,
		       LogMessageFactory* factory, LockingGate* waitGate,
		       LockingGate* exitGate,
		       const std::chrono::system_clock::time_point& deadline);
      ~LogMessageWaiter();

      const std::string& name() const { return name_; }
      bool deadlineExpiredWhileWaiting() const {
	return deadlineExpiredWhileWaiting_;
      }
      const std::string& errors() const { return errors_; }

    private:
      std::string name_;
      LogMessageFactory* factory_;
      LockingGate* waitGate_;
      LockingGate* exitGate_;
      std::chrono::system_clock::time_point deadline_;
      bool deadlineExpiredWhileWaiting_;
      std::string errors_;
      std::unique_ptr<std::thread> thread_;

      void run_();
    };

  }
}
#endif

