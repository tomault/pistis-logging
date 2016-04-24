#include "TrackingLogMessageFactory.hpp"
#include <algorithm>
#include <sstream>

using namespace pistis::logging;

TrackingLogMessageFactory::TrackingLogMessageFactory(size_t initialMessageSize,
						     size_t maxMessageSize):
    initialMsgSize_(initialMessageSize), maxMsgSize_(maxMessageSize),
    issuedMsgs_(), releasedMsgs_(), sync_() {
  // Intentionally left blank
}

TrackingLogMessageFactory::~TrackingLogMessageFactory() {
  clear();
}

void TrackingLogMessageFactory::clear() {
  std::unique_lock<std::mutex> lock(sync_);
  std::for_each(releasedMsgs_.begin(), releasedMsgs_.end(),
		[](LogMessage* msg) { delete msg; });
}

LogMessage* TrackingLogMessageFactory::get_() {
  std::unique_lock<std::mutex> lock(sync_);
  issuedMsgs_.push_back(new LogMessage(initialMessageSize(), maxMessageSize()));
  return issuedMsgs_.back();
}

void TrackingLogMessageFactory::release_(LogMessage* msg) {
  std::unique_lock<std::mutex> lock(sync_);
  // Verify that the message was in fact issued by this factory and has not
  // already been released.
  auto i= std::find(issuedMsgs_.begin(), issuedMsgs_.end(), msg);
  if (i == issuedMsgs_.end()) {
    std::ostringstream details;
    details << "Attempt to release LogMessage 0x" << msg
	    << " which was never allocated";
    errors_.push_back(details.str());
  } else if (std::find(releasedMessages().begin(), releasedMessages().end(),
		       msg) != releasedMessages().end()) {
    std::ostringstream details;
    details << "Attempt to release LogMessage 0x" << msg << "(\"" << *msg
	    << "\") multiple times";
    errors_.push_back(details.str());
  } else {
    issuedMsgs_.erase(i);
    releasedMsgs_.push_back(msg);
  }
}
