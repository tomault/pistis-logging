#ifndef __PISTIS__LOGGING__LOG_HPP__
#define __PISTIS__LOGGING__LOG_HPP__

#include <pistis/logging/LogStream.hpp>

namespace pistis {
  namespace logging {

    class Log {
    public:
      Log(const Log&)= delete;
      Log& operator=(const Log&)= delete;

      const std::string& destination() const { return destination_; }
      LogLevel logLevel() const { return logLevel_; }

      bool isEnabled(LogLevel l) const { return logLevel_ <= l; }
      bool isTraceEnabled() const { return isEnabled(LogLevel::TRACE); }
      bool isDebugEnabled() const { return isEnabled(LogLevel::DEBUG); }
      bool isInfoEnabled() const { return isEnabled(LogLevel::INFO); }
      bool isWarnEnabled() const { return isEnabled(LogLevel::WARN); }
      bool isErrorEnabled() const { return isEnabled(LogLevel::ERROR); }
	
      LogStream<char> log(LogLevel l) const {
	return LogStream<char>(*msgFactory_, *msgReceiver_, destination(),
			       l, isEnabled(l));
      }
      LogStream<char> trace() { return log(LogLevel::TRACE); }
      LogStream<char> debug() const { return log(LogLevel::DEBUG); }
      LogStream<char> info() const { return log(LogLevel::INFO); }
      LogStream<char> warn() const { return log(LogLevel::WARN); }
      LogStream<char> error() const { return log(LogLevel::ERROR); }

      LogStream<wchar_t> wlog(LogLevel l) const {
	return LogStream<wchar_t>(*msgFactory_, *msgReceiver_, destination(),
				  l, isEnabled(l));
      }
      LogStream<wchar_t> wtrace() const { return wlog(LogLevel::TRACE); }
      LogStream<wchar_t> wdebug() const { return wlog(LogLevel::DEBUG); }
      LogStream<wchar_t> winfo() const { return wlog(LogLevel::INFO); }
      LogStream<wchar_t> wwarn() const { return wlog(LogLevel::WARN); }
      LogStream<wchar_t> werror() const { return wlog(LogLevel::ERROR); }

    protected:
      Log(LogMessageFactory* msgFactory, LogMessageReceiver* msgReceiver,
	  const std::string& destination, LogLevel logLevel);
      ~Log() = default;

      void setLogLevel_(LogLevel l) { logLevel_ = l; }
      LogMessageFactory* getFactory_() const { return msgFactory_; }
      LogMessageReceiver* getReceiver_() const { return msgReceiver_; }

    private:
      /** @brief Log obtains LogMessage's from here */
      LogMessageFactory* msgFactory_;

      /** @brief Writes log messages created by this Log */
      LogMessageReceiver* msgReceiver_;

      /** @brief Name of the Log's target */
      std::string destination_;

      /** @brief The current logging level */
      LogLevel logLevel_;
    };
      
  }
}
#endif

