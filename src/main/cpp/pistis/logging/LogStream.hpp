#ifndef __PISTIS__LOGGING__LOGSTREAM_HPP__
#define __PISTIS__LOGGING__LOGSTREAM_HPP__

#include <pistis/logging/LogStreamBuffer.hpp>
#include <pistis/logging/LogLevel.hpp>
#include <iostream>
#include <memory>

namespace pistis {
  namespace logging {

    template <typename CharT, typename TraitsT=std::char_traits<CharT> >
    class LogStream {
    private:
      class Impl {
      public:
	Impl(LogMessageFactory& factory, LogMessageReceiver& receiver,
	     const std::string& destination, LogLevel logLevel,
	     bool enabled):
	    buffer_(factory, receiver, destination, logLevel), out_(&buffer_),
	    enabled_(enabled) {
	  // Intentionally left blank
	}
	~Impl() { }

	const std::string& destination() const {
	  return buffer_.destination();
	}
	LogLevel logLevel() const { return buffer_.logLevel(); }
	bool enabled() const { return enabled_; }

	template <typename T>
	void write(const T& value) {
	  if (enabled()) {
	    out_ << value;
	  }
	}

	void write(const char* data, std::streamsize n) {
	  if (enabled()) {
	      out_.write(data, n);
	  }
	}

	void flush() {
	  if (enabled()) {
	    out_.flush();
	  }
	}

      private:
	LogStreamBuffer<CharT, TraitsT> buffer_;
	std::basic_ostream<CharT, TraitsT> out_;
	bool enabled_;
      };

    public:
      LogStream(LogMessageFactory& factory, LogMessageReceiver& receiver,
		const std::string& destination, LogLevel logLevel,
		bool enabled):
	  impl_(new Impl(factory, receiver, destination, logLevel, enabled)) {
	// Intentionally left blank
      }
      LogStream(const LogStream& other) = delete;
      LogStream(LogStream&& other):
	  impl_(other.impl_.release()) {
	// Intentionally left blank
      }
      ~LogStream() { }

      const std::string& destination() const { return impl_->destination(); }
      LogLevel logLevel() const { return impl_->logLevel(); }
      bool enabled() const { return impl_->enabled(); }

      template <typename ObjectT>
      const LogStream& write(const ObjectT& o) const {
	impl_->write(o);
	return *this;
      }

      const LogStream& write(const CharT* data, std::streamsize n) const {
	impl_->write(data, n);
	return *this;
      }

      const LogStream& flush() const {
	impl_->flush();
	return *this;
      }

      LogStream& operator=(const LogStream&) = delete;
      LogStream& operator=(LogStream&& other) {
	impl_ = std::move(other.impl_);
	return *this;
      }

    private:
      std::unique_ptr<Impl> impl_;
    };

    template <typename CharT, typename TraitsT, typename ObjectT>
    inline const LogStream<CharT, TraitsT>& operator<<(
        const LogStream<CharT, TraitsT>& out,
	const ObjectT& o
    ) {
      return out.write(o);
    }

    template <typename CharT, typename TraitsT>
    inline const LogStream<CharT, TraitsT>& operator<<(
	const LogStream<CharT, TraitsT>& out,
	std::basic_ios<CharT, TraitsT>&
	    (*manip)(std::basic_ios<CharT, TraitsT>&)
    ) {
      return out.write(manip);
    }

    template <typename CharT, typename TraitsT>
    inline const LogStream<CharT, TraitsT>& operator<<(
       const LogStream<CharT, TraitsT>& out,
       std::basic_ostream<CharT, TraitsT>&
           (*manip)(std::basic_ostream<CharT, TraitsT>&)
    ) {
      return out.write(manip);
    }

  }
}

#endif

