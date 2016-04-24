#ifndef __PISTIS__LOGGING__HELPERS__TRACKINGLOGMESSGERECEIVER_HPP__
#define __PISTIS__LOGGING__HELPERS__TRACKINGLOGMESSGERECEIVER_HPP__

#include <pistis/logging/LogMessage.hpp>
#include <pistis/logging/LogMessageFactory.hpp>
#include <pistis/logging/LogMessageReceiver.hpp>
#include <vector>

namespace pistis {
  namespace logging {

    /** @brief A LogMessageReceiver that tracks the messages sent back to it.
     *
     *  The TrackingLogMessageReceiver buffers the messages it receives,
     *  so they can be inspected for correctness, and then releases them to
     *  its LogMessageFactory when it is destroyed or when its
     *  <tt>flush()</tt> method is called.
     */
    class TrackingLogMessageReceiver : public LogMessageReceiver {
    public:
      TrackingLogMessageReceiver(LogMessageFactory* factory);
      virtual ~TrackingLogMessageReceiver();

      const std::vector<LogMessage*>& messages() const { return msgs_; }
      virtual void receive(LogMessage* msg);
      void flush();

    private:
      LogMessageFactory* factory_; ///< Where log messages go when they die
      std::vector<LogMessage*> msgs_; ///< Buffered messages
    };

  }
}
#endif

