#include "LogMessagePool.hpp"
#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace pistis::logging;

LogMessagePool::LogMessagePool(size_t initialMessageSize,
			       size_t maxMessageSize,
			       size_t maxReturnedMessageSize,
			       uint32_t initialPoolSize,
			       uint32_t maxPoolSize):
    initialMessageSize_(initialMessageSize), maxMessageSize_(maxMessageSize),
    maxReturnedMessageSize_(maxReturnedMessageSize), maxPoolSize_(maxPoolSize),
    pool_() {
  pool_.reserve(initialPoolSize);
  while (pool_.size() < initialPoolSize) {
    pool_.push_back(new LogMessage(initialMessageSize_, maxMessageSize_));
  }
}

LogMessagePool::~LogMessagePool() {
  for (auto i : pool_) {
    delete i;
  }
}

LogMessage* LogMessagePool::get_() {
  LogMessage* m= popMessage_();
  if (!m) {
    // Pool is empty
    m = createMessage_();
  }
  return m;
}

void LogMessagePool::release_(LogMessage* msg) {
  if ((msg->capacity() > maxReturnedMessageSize()) || !pushMessage_(msg)) {
    // Message is too big to be returned or the pool is full
    // std::cout << "Destroying " << msg << std::endl;
    releaseMessage_(msg);
  }
}

bool LogMessagePool::pushMessage_(LogMessage* msg) {
  if (!msg) {
    return false;  // Cannot push a null message
  }

  std::unique_lock<std::mutex> lock(sync_);
  if (pool_.size() == maxPoolSize_) {
    return false;
  }
  pool_.push_back(msg);
  return true;
}

LogMessage* LogMessagePool::popMessage_() {
  std::unique_lock<std::mutex> lock(sync_);
  LogMessage* m = nullptr;

  if (!pool_.empty()) {
    m = pool_.back();
    pool_.pop_back();
    m->setEnd(m->begin());
  }
  return m;
}

LogMessage* LogMessagePool::createMessage_() {
  return new LogMessage(initialMessageSize(), maxMessageSize());
}

void LogMessagePool::releaseMessage_(LogMessage* msg) {
  delete msg;
}
