#include "LockingGate.hpp"

using namespace pistis::logging;

const std::chrono::system_clock::time_point LockingGate::WAIT_FOREVER_;

LockingGate::LockingGate():
    isOpen_(false), numAtGate_(0), numArrived_(0), numPassedThrough_(0),
    numWaitingForThreads_(0), numWaitingUntilClear_(0),
    numWaitingUntilArrived_(0), numWaitingForPassThrough_(0), sync_(),
    gateCv_(), waitForThreadsCv_(), waitUntilClearCv_(), arrivedCv_(),
    passThroughCv_() {
  // Intentionally left blank
}

LockingGate::~LockingGate() {
  if (!isOpen_) {
    open();
    waitUntilClear(std::chrono::system_clock::time_point());
  }
}

void LockingGate::open() {
  std::unique_lock<std::mutex> lock(sync_);
  if (!isOpen_) {
    isOpen_= true;
    if (numAtGate_) {
      gateCv_.notify_all();
    }
    if (numWaitingForThreads_) {
      waitForThreadsCv_.notify_all();
    }
  }
}

bool LockingGate::wait(const std::chrono::system_clock::time_point& deadline) {
  std::unique_lock<std::mutex> lock(sync_);

  ++numArrived_;
  if (numWaitingUntilArrived_) {
    arrivedCv_.notify_all();
  }
  if (numWaitingForThreads_ && !isOpen_) {
    waitForThreadsCv_.notify_all();
  }
  if (deadline == WAIT_FOREVER_) {
    while (!isOpen_) {
      ++numAtGate_;
      gateCv_.wait(lock);
      --numAtGate_;
    }
  } else {
    while (!isOpen_ && (std::chrono::system_clock::now() < deadline)) {
      ++numAtGate_;
      gateCv_.wait_until(lock, deadline);
      --numAtGate_;
    }
  }

  ++numPassedThrough_;
  if (numWaitingUntilClear_ && !numAtGate_) {
    waitUntilClearCv_.notify_all();
  }
  if (numWaitingForPassThrough_) {
    passThroughCv_.notify_all();
  }
  return isOpen_;
}

bool LockingGate::waitForThreads(
    size_t n, const std::chrono::system_clock::time_point& deadline
) {
  std::unique_lock<std::mutex> lock(sync_);

  if (deadline == WAIT_FOREVER_) {
    while (!isOpen_ && (numAtGate_ < n)) {
      ++numWaitingForThreads_;
      waitForThreadsCv_.wait(lock);
      --numWaitingForThreads_;
    }
  } else {
    while (!isOpen_ && (numAtGate_ < n) &&
	   (std::chrono::system_clock::now() < deadline)) {
      ++numWaitingForThreads_;
      waitForThreadsCv_.wait_until(lock, deadline);
      --numWaitingForThreads_;
    }
  }
  return numAtGate_ >= n;
}

bool LockingGate::waitUntilClear(
    const std::chrono::system_clock::time_point& deadline
) {
  std::unique_lock<std::mutex> lock(sync_);

  if (deadline == WAIT_FOREVER_) {
    while (numAtGate_) {
      ++numWaitingUntilClear_;
      waitUntilClearCv_.wait(lock);
      --numWaitingUntilClear_;
    }
  } else {
    while (numAtGate_ && (std::chrono::system_clock::now() < deadline)) {
      ++numWaitingUntilClear_;
      waitUntilClearCv_.wait_until(lock, deadline);
      --numWaitingUntilClear_;
    }
  }
  return !numAtGate_;
}

bool LockingGate::waitUntilArrived(
    size_t n, const std::chrono::system_clock::time_point& deadline
) {
  std::unique_lock<std::mutex> lock(sync_);

  if (deadline == WAIT_FOREVER_) {
    while (numArrived_ < n) {
      ++numWaitingUntilArrived_;
      arrivedCv_.wait(lock);
      --numWaitingUntilArrived_;
    }
  } else {
    while ((numArrived_ < n) &&
	   (std::chrono::system_clock::now() < deadline)) {
      ++numWaitingUntilArrived_;
      arrivedCv_.wait_until(lock, deadline);
      --numWaitingUntilArrived_;
    }
  }
  return numArrived_ >= n;
}

bool LockingGate::waitUntilPassedThrough(
    size_t n, const std::chrono::system_clock::time_point& deadline
) {
  std::unique_lock<std::mutex> lock(sync_);

  if (deadline == WAIT_FOREVER_) {
    while (numPassedThrough_ < n) {
      ++numWaitingForPassThrough_;
      passThroughCv_.wait(lock);
      --numWaitingForPassThrough_;
    }
  } else {
    while ((numPassedThrough_ < n) &&
	   (std::chrono::system_clock::now() < deadline)) {
      ++numWaitingForPassThrough_;
      passThroughCv_.wait_until(lock, deadline);
      --numWaitingForPassThrough_;
    }
  }
  return numPassedThrough_ >= n;
}
