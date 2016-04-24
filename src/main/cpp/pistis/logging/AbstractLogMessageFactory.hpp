#ifndef __PISTIS__LOGGING__ABSTRACTLOGMESSAGEFACTORY_HPP__
#define __PISTIS__LOGGING__ABSTRACTLOGMESSAGEFACTORY_HPP__

#include <pistis/logging/LogMessageFactory.hpp>
#include <atomic>

namespace pistis {
  namespace logging {

    class AbstractLogMessageFactory : public LogMessageFactory {
    public:
      virtual ~AbstractLogMessageFactory() { }

      /** @brief Returns the number of messages created by get() but not yet
       *           returned to the factory by release().
       */
      size_t numMessagesActive() const {
	return numMessagesActive_.load(std::memory_order_acquire);
      }

      /** @brief Returns the number of threads waiting until all messages
       *           have been returned to the factory.
       */
      size_t numWaitingUntilAllReturned() const {
	return numWaitingUntilAllReturned_.load(std::memory_order_acquire);
      }

      /** @brief Obtain a new LogMessage */
      virtual LogMessage* get();

      /** @brief Return a LogMessage to the factory
       *
       *  Applications should not delete messages themselves, but must call
       *  this method when done using a message.  The log factory may
       *  elect to reuse messages rather than deleting them.
       */
      virtual void release(LogMessage* msg);

      /** @brief Wait until all messages have been returned to the factory,
       *         or the deadline arrives.
       *
       *  This message is typically called when the logging system shuts
       *  down to wait until all threads currently writing a log messag
       *  have completed their task.
       */
      virtual bool waitUntilAllReturned(
          const std::chrono::system_clock::time_point& deadline
      );

    protected:
      AbstractLogMessageFactory();
      virtual LogMessage* get_() = 0;
      virtual void release_(LogMessage* msg) = 0;

    private:
      /** @brief Number of messages allocated by the factory but not yet
       *          released.
       */
      std::atomic_uint_fast64_t numMessagesActive_;

      /** @brief Number of threads waiting until all messages have been
       *         returned to the factory
       */
      std::atomic_uint_fast64_t numWaitingUntilAllReturned_;

      /** @brief How much time to spend between checks for all messages to
       *         return to the factory in waitUntilAllReturned()
       */
      static const std::chrono::milliseconds SPIN_DELAY_;
    };
    
  }
}
#endif

