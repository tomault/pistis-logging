#include "TrackingLogMessageReceiver.hpp"
#include <algorithm>

using namespace pistis::logging;

TrackingLogMessageReceiver::TrackingLogMessageReceiver(
    LogMessageFactory* factory
):
    factory_(factory) {
  // Intentionally left blank
}

TrackingLogMessageReceiver::~TrackingLogMessageReceiver() {
  flush();
}

void TrackingLogMessageReceiver::receive(LogMessage* msg) {
  if (msg) {
    msgs_.push_back(msg);
  }
}

void TrackingLogMessageReceiver::flush() {
  std::for_each(msgs_.begin(), msgs_.end(),
		[this](LogMessage* msg) { factory_->release(msg); });
}
