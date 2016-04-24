#include "LogMessageConsumer.hpp"

using namespace pistis::logging;

LogMessageConsumer::LogMessageConsumer(
    const std::string& name, LogMessageFactory* factory, LockingGate* getGate,
    LockingGate* releaseGate, LockingGate* exitGate,
    const std::chrono::system_clock::time_point& deadline
):
    name_(name), factory_(factory), message_(nullptr), getGate_(getGate),
    releaseGate_(releaseGate), exitGate_(exitGate), deadline_(deadline),
    errors_(), thread_(new std::thread([this]() { this->run_(); })) {
  // Intentionally left blank
}

LogMessageConsumer::~LogMessageConsumer() {
  if (!getGate_->isOpen()) {
    getGate_->open();
  }
  if (!releaseGate_->isOpen()) {
    releaseGate_->open();
  }
  if (!exitGate_->isOpen()) {
    exitGate_->open();
  }
  thread_->join();
}

void LogMessageConsumer::run_() {
  if (!getGate_->wait(deadline_)) {
    // Deadline expired, so abandon thread
    errors_ = "Deadline expired while waiting at GET gate";
    return;
  }
  message_= factory_->get();
  if (!message_) {
    // Failed to receive message, so abandon thread
    errors_ = "Retrieved a null message";
    return;
  }
  if (!releaseGate_->wait(deadline_)) {
    errors_ = "Deadline expired while waiting at RELEASE gate";
    return;
  }
  factory_->release(message_);
  message_ = nullptr;
  exitGate_->wait(deadline_);
}
