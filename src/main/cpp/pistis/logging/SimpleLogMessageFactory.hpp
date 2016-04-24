#ifndef __PISTIS__LOGGING__SIMPLELOGMESSAGEFACTORY_HPP__
#define __PISTIS__LOGGING__SIMPLELOGMESSAGEFACTORY_HPP__

#include <pistis/logging/AbstractLogMessageFactory.hpp>

namespace pistis {
  namespace logging {

    class SimpleLogMessageFactory : public AbstractLogMessageFactory {
    public:
      SimpleLogMessageFactory(size_t initialMessageSize,
			      size_t maxMessageSize);
      virtual ~SimpleLogMessageFactory();

      size_t initialMessageSize() const { return initialMessageSize_; }
      size_t maxMessageSize() const { return maxMessageSize_; }

    protected:
      virtual LogMessage* get_() override;
      virtual void release_(LogMessage* msg) override;
	
    private:
      size_t initialMessageSize_;
      size_t maxMessageSize_;
    };

  }
}
#endif

