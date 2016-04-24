#include "TestingLogMessageReceiver.hpp"

using namespace pistis::logging;

TestingLogMessageReceiver::TestingLogMessageReceiver(
    LogMessageFactory* factory
): factory_(factory) {
  // Intentionally left blank
}

void TestingLogMessageReceiver::receive(LogMessage* msg) {
  if (msg) {
    factory_->release(msg);
  }
}
