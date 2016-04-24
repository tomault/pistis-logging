#ifndef __PISTIS__LOGGING__LOGMESSAGE_HPP__
#define __PISTIS__LOGGING__LOGMESSAGE_HPP__

#include <pistis/logging/LogLevel.hpp>
#include <iostream>
#include <stdlib.h>

namespace pistis {
  namespace logging {

    class LogMessage {
    public:
      LogMessage(size_t capacity);
      LogMessage(size_t initialCapacity, size_t maximumCapacity);
      LogMessage(const LogMessage& other)= delete;
      LogMessage(LogMessage&& other);
      virtual ~LogMessage();

      bool empty() const { return end_ == data_; }
      bool full() const { return end_ == eos_; }
      bool atMaxCapacity() const { return capacity() == maxCapacity(); }

      size_t size() const { return (size_t)(end_ - data_); }
      size_t available() const { return (size_t)(eos_ - end_); }
      size_t capacity() const { return (size_t)(eos_ - data_); }
      size_t maxCapacity() const { return maxCapacity_; }
	
      LogLevel logLevel() const { return logLevel_; }
      const std::string& destination() const { return destination_; }
      void setLogLevel(LogLevel l) { logLevel_ = l; }
      void setDestination(const std::string& destination) {
	destination_ = destination;
      }

      char* begin() const { return data_; }
      char* end() const { return end_; }
      char* eos() const { return eos_; }

      void setEnd(char* newEnd) { end_ = newEnd; }
      virtual size_t increaseCapacity(size_t desiredCapacity);

      LogMessage& operator=(const LogMessage&) = delete;
      LogMessage& operator=(LogMessage&& other);

    private:
      char* data_;
      char* end_;
      char* eos_;
      size_t maxCapacity_;
      LogLevel logLevel_;
      std::string destination_;

      /** @brief Increase the size of the buffer.
       *
       *  Requires that newSize is equal to or larger than capacity().
       *
       *  @param newSize  New size of the buffer; must be &gt;= capacity()
       *  @param copyData If true, copy data from the current buffer to
       *                    the new one
       */
      void increaseBufferSize_(size_t newSize, bool copyData);
    };

    inline std::ostream& operator<<(std::ostream& out,
				    const LogMessage& msg) {
      out.write(msg.begin(), msg.size());
      return out;
    }

  }
}
#endif
