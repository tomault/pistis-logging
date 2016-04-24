#include "LogMessage.hpp"
#include <memory>
#include <string.h>

using namespace pistis::logging;

LogMessage::LogMessage(size_t capacity):
    data_(new char[capacity]), end_(data_), eos_(data_ + capacity),
    maxCapacity_(capacity), logLevel_(), destination_() {
  // Intentionally left blank
}

LogMessage::LogMessage(size_t initialCapacity, size_t maximumCapacity):
    data_(new char[initialCapacity]), end_(data_),
    eos_(data_ + initialCapacity), maxCapacity_(maximumCapacity),
    logLevel_(), destination_() {
  // Intentionally left blank
}

LogMessage::LogMessage(LogMessage&& other):
    data_(other.data_), end_(other.end_), eos_(other.eos_),
    maxCapacity_(other.maxCapacity()), logLevel_(other.logLevel()),
    destination_(std::move(other.destination_)) {
  other.data_ = nullptr;
  other.end_ = nullptr;
  other.eos_ = nullptr;
  other.maxCapacity_ = 0;
}

LogMessage::~LogMessage() {
  delete data_;
}

size_t LogMessage::increaseCapacity(size_t desiredCapacity) {
  size_t targetCapacity=
    std::min(std::max(desiredCapacity, capacity()), maxCapacity());
  if (targetCapacity > capacity()) {
    increaseBufferSize_(targetCapacity, true);
  }
  return capacity();
}

LogMessage& LogMessage::operator=(LogMessage&& other) {
  if (this != &other) {
    data_ = other.data_; other.data_ = nullptr;
    end_ = other.end_; other.end_ = nullptr;
    eos_ = other.eos_; other.eos_ = nullptr;
    maxCapacity_ = other.maxCapacity_; other.maxCapacity_ = 0;
    logLevel_ = other.logLevel_;
    destination_ = std::move(other.destination_);
  }
  return *this;
}

void LogMessage::increaseBufferSize_(size_t newSize, bool copyData) {
  std::unique_ptr<char[]> newData(new char[newSize]);
  char* tmp= data_;
  size_t oldSize= size();
  if (copyData && oldSize) {
    memcpy(newData.get(), data_, oldSize);
    end_ = newData.get() + oldSize;
  } else {
    end_ = newData.get();
  }

  // Swap _data and newData.  newData will clean up old buffer when control
  // exits this block
  data_ = newData.release();
  eos_ = data_ + newSize;
  newData.reset(tmp);
}
