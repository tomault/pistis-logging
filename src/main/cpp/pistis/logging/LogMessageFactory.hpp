#ifndef __PISTIS__LOGGING__LOGMESSAGEFACTORY_HPP__
#define __PISTIS__LOGGING__LOGMESSAGEFACTORY_HPP__

#include <pistis/logging/LogMessage.hpp>
#include <chrono>

namespace pistis {
  namespace logging {

    /** @brief Interface for classes that create and destroy LogMessage
     *         instances.
     *
     *  The pistis logging API delegates the creation and destruction of
     *  LogMessage instances to objects that implement the LogMessageFactory
     *  interface.  The logging implementation injects a LogMessageFactory
     *  into a Log whenever one is created.  The Log passes the
     *  LogMessageFactory to a LogStream whenever it creates one, and
     *  the LogStream in turn passes the LogMessageFactory to its
     *  LogStreamBuffer.  The LogStreamBuffer uses the LogMessageFactory
     *  to create LogMessage's as needed.  The logging implementation is
     *  responsible for managing the lifetime of a LogMessageFactory.
     *  The logging API itself neither creates nor destroys LogMessageFactory
     *  instances.
     *
     *  As a convenience for logging implementations, the logging API
     *  provides two LogMessageFactory implementations:
     *  SimpleLogMessageFactory and LogMessagePool.  The former allocates
     *  and destroys LogMessage instances on demand, while the latter
     *  maintains a pool of messages it draws from, allocating and destroying
     *  new messages only as needed.
     */
    class LogMessageFactory {
    public:
      virtual ~LogMessageFactory() { }

      /** @brief Obtain a LogMessage from the factory
       *
       *  The message will be empty.  Messages will be created without any
       *  delays.
       *
       *  @returns A fresh LogMessage
       *  @throws std::bad_alloc if the factory cannot allocate a new message
       */
      virtual LogMessage* get() = 0;

      /** @brief Return a message to the factory.
       *
       *  Applications should use this method to release messages they
       *  no longer need, rather than deleting them.  Deleting a message
       *  will probably cause waitUntilFull() to block indefinitely waiting
       *  for the deleted messge to be returned.
       *
       *  @param msg  The message to release.  Null values will be ignored.
       *  @pre msg was obtained by calling <code>this->get()</code>.
       *  @throws  Does not throw
       */
      virtual void release(LogMessage* msg) = 0;

      /** @brief Wait until all LogMessages have been returned to the factory
       *
       *  Blocks until either (a) all messages obtained from the factory by
       *  calling get() have been returned by calling release() or (b)
       *  the current time exceeds the deadline.
       *
       *  @param deadline  Time to wait until.  Pass
       *                     std::chrono::system_clock::time_point() to
       *                     wait indefinitely.
       *  @returns  True if all messages have been returned to the factory.
       *            False if the deadline passed without all messages being
       *              returned.
       *  @throws  std::system_error if a system error occurs while waiting
       */
      virtual bool waitUntilAllReturned(
	  const std::chrono::system_clock::time_point& deadline
      ) = 0;

      /** @brief Wait until all LogMessages have been returned to the factory
       *
       *  Blocks indefinitely until all messages obtained from the factory
       *  by calling get() have been returned by calling release.  Equivalent
       *  to calling <code>waitUntilFull(std::not_a_date_time)</code>.
       *
       *  @see waitUntilAllReturned(const std::chrono::system_clock::time_point&)
       *  @throws std::system_error if a system error occurs while waiting
       */
      virtual bool waitUntilAllReturned() {
	return waitUntilAllReturned(std::chrono::system_clock::time_point());
      }
    };
      
  }
}
#endif

