#include "Log.hpp"

using namespace pistis::logging;

Log::Log(LogMessageFactory* msgFactory, LogMessageReceiver* msgReceiver,
	 const std::string& destination, LogLevel logLevel):
    msgFactory_(msgFactory), msgReceiver_(msgReceiver),
    destination_(destination), logLevel_(logLevel) {
  // Intentionally left blank
}

