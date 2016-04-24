#ifndef __PISTIS__LOGGING__LOGMESSAGEPOOL_HPP__
#define __PISTIS__LOGGING__LOGMESSAGEPOOL_HPP__

#include <pistis/logging/AbstractLogMessageFactory.hpp>
#include <algorithm>
#include <mutex>
#include <vector>
#include <stdint.h>

namespace pistis {
  namespace logging {

    class LogMessagePool : public AbstractLogMessageFactory {
    public:
      LogMessagePool(size_t initialMessageSize, size_t maxMessageSize,
		     size_t maxReturnedMessageSize,
		     uint32_t initialPoolSize, uint32_t maxPoolSize);
      virtual ~LogMessagePool();

      size_t initialMessageSize() const { return initialMessageSize_; }
      size_t maxMessageSize() const { return maxMessageSize_; }
      size_t maxReturnedMessageSize() const { return maxReturnedMessageSize_; }
      uint32_t maxPoolSize() const { return maxPoolSize_; }

    protected:
      bool pushMessage_(LogMessage* msg);
      LogMessage* popMessage_();

      virtual LogMessage* createMessage_();
      void releaseMessage_(LogMessage* msg);

      virtual LogMessage* get_();
      virtual void release_(LogMessage* msg);

      size_t numMessagesInPool_() const { return pool_.size(); }

      template <typename OutputIterator>
      OutputIterator getMessagesInPool_(const OutputIterator& out) const {
	return std::copy(pool_.begin(), pool_.end(), out);
      }
      
    private:
      size_t initialMessageSize_;
      size_t maxMessageSize_;
      size_t maxReturnedMessageSize_;
      size_t maxPoolSize_;
      std::vector<LogMessage*> pool_;
      std::mutex sync_;
    };

  }
}
#endif

