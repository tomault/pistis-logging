#include "AbstractLogMessageFactory.hpp"
#include <thread>

using namespace pistis::logging;

const std::chrono::milliseconds AbstractLogMessageFactory::SPIN_DELAY_(100);

AbstractLogMessageFactory::AbstractLogMessageFactory():
    numMessagesActive_(0), numWaitingUntilAllReturned_(0) {
}

LogMessage* AbstractLogMessageFactory::get() {
  LogMessage* msg= get_();
  ++numMessagesActive_;
  return msg;
}


void AbstractLogMessageFactory::release(LogMessage* msg) {
  if (msg) {
    release_(msg);
    --numMessagesActive_;
  }
}

bool AbstractLogMessageFactory::waitUntilAllReturned(
    const std::chrono::system_clock::time_point& deadline
) {
  size_t numActive = numMessagesActive();
  if (numActive) {
    ++numWaitingUntilAllReturned_;
    // Since the typical usage pattern acquires a message and returns it
    // within a relatively short period of time, and because
    // waitUntilAllReturned() is typically called as part of shutdown code,
    // spinning until the condition is true is tolerable.
    while ((std::chrono::system_clock::now() < deadline) && numActive) {
      std::this_thread::sleep_until(
          std::chrono::system_clock::now() + SPIN_DELAY_
      );
      numActive = numMessagesActive();
    }
    --numWaitingUntilAllReturned_;
  }
  return !numActive;
}
