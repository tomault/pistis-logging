#ifndef __PISTIS__LOGGING__HELPERS__TRACKINGLOGMESSAGEFACTORY_HPP__
#define __PISTIS__LOGGING__HELPERS__TRACKINGLOGMESSAGEFACTORY_HPP__

#include <pistis/logging/AbstractLogMessageFactory.hpp>
#include <pistis/logging/LogMessage.hpp>
#include <string>
#include <mutex>
#include <vector>
#include <stddef.h>

#include "Join.hpp"

namespace pistis {
  namespace logging {

    /** @brief LogMessageFactory that keeps track of which messages have
     *         been issued and returned.
     */
    class TrackingLogMessageFactory : public AbstractLogMessageFactory {
    public:
      TrackingLogMessageFactory(size_t initialMessageSize,
				size_t maxMessageSize);
      virtual ~TrackingLogMessageFactory();

      size_t initialMessageSize() const { return initialMsgSize_; }
      size_t maxMessageSize() const { return maxMsgSize_; }

      const std::vector<LogMessage*>& issuedMessages() const {
	return issuedMsgs_;
      }
      const std::vector<LogMessage*>& releasedMessages() const {
	return releasedMsgs_;
      }
      bool hasErrors() const { return errors_.size(); }
      std::string errorDetails() const {
	return join(errors_.begin(), errors_.end(), "\n");
      }
      void clear();

    protected:
      virtual LogMessage* get_();
      virtual void release_(LogMessage* msg);

    private:
      size_t initialMsgSize_; ///< Initial size of a LogMessage
      size_t maxMsgSize_;  ///< Maximum size of a LogMessage
      std::vector<LogMessage*> issuedMsgs_; ///< Currently allocated messages
      std::vector<LogMessage*> releasedMsgs_; ///< Released messages
      std::vector<std::string> errors_; ///< Any errors that have occurred
      std::mutex sync_; ///< Synchronizes access to message factory
    };

  }
}

#endif
