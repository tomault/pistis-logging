#include "TestingLog.hpp"

using namespace pistis::logging;

TestingLog::TestingLog(LogMessageFactory* msgFactory,
		       LogMessageReceiver* msgReceiver,
		       const std::string& destination,
		       LogLevel logLevel):
    Log(msgFactory, msgReceiver, destination, logLevel) {
  // Intentionally left blank
}
