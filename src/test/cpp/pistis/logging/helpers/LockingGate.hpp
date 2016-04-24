#ifndef __PISTIS__LOGGING__HELPERS__LOCKINGGATE_HPP__
#define __PISTIS__LOGGING__HELPERS__LOCKINGGATE_HPP__

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stddef.h>

namespace pistis {
  namespace logging {

    class LockingGate {
    public:
      LockingGate();
      virtual ~LockingGate();
      
      bool isOpen() const { return isOpen_; }
      void open();
      bool wait(
	  const std::chrono::system_clock::time_point& deadline
      );
      bool waitForThreads(
	  size_t n, const std::chrono::system_clock::time_point& deadline
      );
      bool waitUntilClear(
	  const std::chrono::system_clock::time_point& deadline
      );
      bool waitUntilArrived(
	  size_t n, const std::chrono::system_clock::time_point& deadline
      );
      bool waitUntilPassedThrough(
	  size_t n, const std::chrono::system_clock::time_point& deadline
      );

    private:
      bool isOpen_;
      size_t numAtGate_;
      size_t numArrived_;
      size_t numPassedThrough_;
      size_t numWaitingForThreads_;
      size_t numWaitingUntilClear_;
      size_t numWaitingUntilArrived_;
      size_t numWaitingForPassThrough_;
      std::mutex sync_;
      std::condition_variable gateCv_;
      std::condition_variable waitForThreadsCv_;
      std::condition_variable waitUntilClearCv_;
      std::condition_variable arrivedCv_;
      std::condition_variable passThroughCv_;

      static const std::chrono::system_clock::time_point WAIT_FOREVER_;
    };
    
  }
}
#endif
