#include "SimpleLogMessageFactory.hpp"

using namespace pistis::logging;

SimpleLogMessageFactory::SimpleLogMessageFactory(size_t initialMessageSize,
						 size_t maxMessageSize):
  AbstractLogMessageFactory(), initialMessageSize_(initialMessageSize),
  maxMessageSize_(maxMessageSize) {
}

SimpleLogMessageFactory::~SimpleLogMessageFactory() {
}

LogMessage* SimpleLogMessageFactory::get_() {
  return new LogMessage(initialMessageSize(), maxMessageSize());
}

void SimpleLogMessageFactory::release_(LogMessage* msg) {
  delete msg;
}
