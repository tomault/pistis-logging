#include "TestingLogMessagePool.hpp"

using namespace pistis::logging;

TestingLogMessagePool::TestingLogMessagePool(
    size_t initialMessageSize, size_t maxMessageSize,
    size_t maxReturnedMessageSize, uint32_t initialPoolSize,
    uint32_t maxPoolSize
):
    LogMessagePool(initialMessageSize, maxMessageSize, maxReturnedMessageSize,
 		   initialPoolSize, maxPoolSize) {
  // Intentionally left blank
}



