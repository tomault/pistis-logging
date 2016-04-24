#ifndef __PISTIS__LOGGING__LOGFACTORY_HPP__
#define __PISTIS__LOGGING__LOGFACTORY_HPP__

#include <pistis/logging/Log.hpp>
#include <pistis/logging/LogFactoryImpl.hpp>
#include <mutex>

namespace pistis {
  namespace logging {

    class LogFactory {
    public:
      Log* getLogInstance(const std::string& destination) {
	return impl_->getLog(destination);
      }

      void setLoggingLevel(const std::string& destination,
			   LogLevel logLevel) {
	return impl_->setLogLevel(destination, logLevel);
      }

      static LogFactory* getInstance() {
	std::call_once(onceFlag_, LogFactory::initFactory_);
	return theFactory_;
      }

      static Log* getLog(const std::string& destination) {
	return getInstance()->getLogInstance(destination);
      }

      static void setLogLevel(const std::string& destination,
			      LogLevel level) {
	return getInstance()->setLoggingLevel(destination, level);
      }
	
    protected:
      LogFactory(LogFactoryImpl* impl);
      ~LogFactory();

    private:
      LogFactoryImpl* impl_;

      struct Destructor {
	LogFactory* factory;

	Destructor();
	~Destructor();
      };

      static LogFactory* theFactory_;
      static Destructor destructor_;
      static std::once_flag onceFlag_;

      static void initFactory_();
    };


    extern LogFactoryImpl* createLogFactoryImpl();

  }
}

#endif

