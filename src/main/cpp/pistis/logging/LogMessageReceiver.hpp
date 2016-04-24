#ifndef __PISTIS__LOGGING__LOGMESSAGERECEIVER_HPP__
#define __PISTIS__LOGGING__LOGMESSAGERECEIVER_HPP__

#include <pistis/logging/LogMessage.hpp>

namespace pistis {
  namespace logging {

    /** @brief Transmits LogMessage instances back to the logging
     *         implementation to be recorded.
     *
     *  There are three points of access between the logging API and the
     *  underlying implementation:
     *  <ol>
     *    <li>The LogFactoryImpl, which the API calls to retrieve a Log
     *        instance for a destination.</li>
     *    <li>The LogMessageFactory, which the API calls to create LogMessage
     *        instances that it fills in with a log message.</li>
     *    <li>The LogMessageReceiver, which the API calls to transmit
     *        a filled-in LogMessage back to the implementation, which
     *        presumably records it somewhere.</li>
     *  </ol>
     *
     *  The logging implementation is responsible for injecting the
     *  LogMessageFactory and LogMessageReceiver into the Log instance
     *  before returning the Log instance back to the API in the
     *  LogMessageImpl::getLog(const std::string&) call.
     */
    class LogMessageReceiver {
    public:
      virtual ~LogMessageReceiver() { }

      /** @brief Transmit a LogMessage back to the logging implementation.
       *
       *  The logging API calls this method to send <tt>msg</tt> back to
       *  the logging implementation to be recorded.  After calling this
       *  method, the logging API no longer manipulates <tt>msg</tt>.
       *  The logging implementation is responsible for returning
       *  <tt>msg</tt> back to the LogMessageFactory that created it.
       */
      virtual void receive(LogMessage* msg) = 0;
    };

  }
}

#endif
