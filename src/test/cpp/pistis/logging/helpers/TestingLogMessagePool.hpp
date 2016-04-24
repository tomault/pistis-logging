#ifndef __PISTIS__LOGGING__HELPERS__TESTINGLOGMESSAGEPOOL_HPP__
#define __PISTIS__LOGGING__HELPERS__TESTINGLOGMESSAGEPOOL_HPP__

#include <pistis/logging/LogMessagePool.hpp>

namespace pistis {
  namespace logging {

    class TestingLogMessagePool : public LogMessagePool {
    public:
      TestingLogMessagePool(size_t initialMessageSize, size_t maxMessageSize,
			    size_t maxReturnedMessageSize,
			    uint32_t initialPoolSize, uint32_t maxPoolSize);

      /** @brief Return the number of messages currently in the pool
       *
       *  @returns The number of messages currently in the pool
       */
      size_t numMessagesInPool() const { return numMessagesInPool_(); }

      /** @brief Retrieve the messages currently in the pool
       *
       *  This method is NOT thread safe and should only be invoked when
       *  it is certain no one else is accessing the pool.  It could be
       *  made thread-safe, with an increase in complexity and dynamic
       *  memory allocation.  However, this method exists primarily for
       *  testing purposes and normal consumers of the pool do not need it.
       *
       *  @param out  Where to write the messages currently in the pool.
       *              Must be an output iterator.
       */
      template <typename OutputIterator>
      OutputIterator getMessagesInPool(const OutputIterator& out) const {
	return getMessagesInPool_(out);
      }
    };

  }
}
#endif

