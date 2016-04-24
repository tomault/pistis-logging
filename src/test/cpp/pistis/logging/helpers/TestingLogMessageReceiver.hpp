#ifndef __PISTIS__LOGGING__HELPERS__TESTINGLOGMESSGERECEIVER_HPP__
#define __PISTIS__LOGGING__HELPERS__TESTINGLOGMESSGERECEIVER_HPP__

#include <pistis/logging/LogMessage.hpp>
#include <pistis/logging/LogMessageFactory.hpp>
#include <pistis/logging/LogMessageReceiver.hpp>

namespace pistis {
  namespace logging {

    /** @brief Just returns any messages it receives to its
     *         LogMessageFactory.
     */
    class TestingLogMessageReceiver : public LogMessageReceiver {
    public:
      TestingLogMessageReceiver(LogMessageFactory* factory);
      
      virtual void receive(LogMessage* msg);

    private:
      LogMessageFactory* factory_;  ///< Where messages go when they die :)
    };

  }
}
#endif

