#include "LogMessageWaiter.hpp"

using namespace pistis::logging;

LogMessageWaiter::LogMessageWaiter(
    const std::string& name, LogMessageFactory* factory, LockingGate* waitGate,
    LockingGate* exitGate,
    const std::chrono::system_clock::time_point& deadline
):
    name_(name), factory_(factory), waitGate_(waitGate), exitGate_(exitGate),
    deadline_(deadline), deadlineExpiredWhileWaiting_(false), errors_(),
    thread_(new std::thread([this]() { this->run_(); })) {
  // Intentionally left blank
}

LogMessageWaiter::~LogMessageWaiter() {
  if (!waitGate_->isOpen()) {
    waitGate_->open();
  }
  if (!exitGate_->isOpen()) {
    exitGate_->open();
  }
  thread_->join();
}

void LogMessageWaiter::run_() {
  if (!waitGate_->wait(deadline_)) {
    errors_ = "Deadline expired while waiting at the gate";
    return;
  }
  deadlineExpiredWhileWaiting_ = !factory_->waitUntilAllReturned(deadline_);
  exitGate_->wait(deadline_);
}
