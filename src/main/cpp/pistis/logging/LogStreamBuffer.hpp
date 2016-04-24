#ifndef __PISTIS__LOGGING__LOGSTREAMBUFFER_HPP__
#define __PISTIS__LOGGING__LOGSTREAMBUFFER_HPP__

#include <pistis/logging/LogLevel.hpp>
#include <pistis/logging/LogMessageFactory.hpp>
#include <pistis/logging/LogMessage.hpp>
#include <pistis/logging/LogMessageReceiver.hpp>
#include <string.h>

namespace pistis {
  namespace logging {

    template <typename CharT, typename TraitsT = std::char_traits<CharT> >
    class LogStreamBuffer : public std::basic_streambuf<CharT, TraitsT> {
    public:
      LogStreamBuffer(LogMessageFactory& msgFactory,
		      LogMessageReceiver& receiver,
		      const std::string& destination,
		      LogLevel logLevel):
	  msgFactory_(msgFactory), msgReceiver_(receiver), 
          destination_(destination), logLevel_(logLevel), current_(nullptr) {
	this->setp(nullptr, nullptr);	  
      }
	
      LogStreamBuffer(const LogStreamBuffer&) = delete;

      LogStreamBuffer(LogStreamBuffer&& other):
	  std::basic_streambuf<CharT, TraitsT>(),
	  msgFactory_(other.msgFactory_), msgReceiver_(other.msgReceiver_),
	  destination_(std::move(other.destination_)),
	  logLevel_(other.logLevel_), current_(other.current_) {
	if (current_) {
	  resetStreamBufPtrs_();
	  other.current_= nullptr;
	  other.setp(nullptr, nullptr);
	} else {
	  this->setp(nullptr, nullptr);
	}
      }

      virtual ~LogStreamBuffer() {
	sync();
      }

      const std::string& destination() const { return destination_; }
      LogLevel logLevel() const { return logLevel_; }

      LogStreamBuffer& operator=(const LogStreamBuffer&)= delete;

    protected:
      virtual std::basic_streambuf<CharT, TraitsT>* setbuf(
	  CharT* buffer, std::streamsize n
      ) {
	// Cannot use an external buffer, so attempts to do so are ignored
	return this;
      }

      virtual typename TraitsT::pos_type seekoff(
	  typename TraitsT::off_type off, std::ios_base::seekdir way,
	  std::ios_base::openmode mode= std::ios_base::in|std::ios_base::out
      ) {
	return typename TraitsT::pos_type(this->pptr() - this->pbase());
      }

      virtual typename TraitsT::pos_type seekpos(
	  typename TraitsT::pos_type pos,
	  std::ios_base::openmode mode= std::ios_base::in|std::ios_base::out
      ) {
	return typename TraitsT::pos_type(this->pptr() - this->pbase());
      }

      virtual int sync() {
	if (current_) {
	  current_->setEnd((char*)this->pptr());
	  msgReceiver_.receive(current_);
	  current_ = nullptr;
	  this->setp(nullptr, nullptr);
	}
	return 0;
      }

      virtual std::streamsize showmanyc() {
	return 0;
      }

      virtual std::streamsize xsgetn(CharT* p, std::streamsize n) {
	// Cannot read from LogStreamBuffer
	return 0;
      }

      virtual typename TraitsT::int_type underflow() {
	return TraitsT::eof();
      }

      virtual typename TraitsT::int_type uflow() {
	return TraitsT::eof();
      }

      virtual typename TraitsT::int_type pbackfail(
	  typename TraitsT::int_type c= TraitsT::eof()
      ) {
	return TraitsT::eof();
      }

      virtual std::streamsize xsputn(const CharT* data, std::streamsize n) {
	std::streamsize remaining= n;
	std::streamsize nWritten= 0;
	const CharT* p= data;
	
	while (remaining) {
	  size_t available = (size_t)(this->epptr() - this->pptr());
	  if (!available) {
	    sync();
	    available = (size_t)(this->epptr() - this->pptr());
	  }
	  if (!this->pptr()) {
	    getNewMessage_();
	    available = (size_t)(this->epptr() - this->pptr());
	  }
	  if (available < remaining) {
	    current_->setEnd((char*)this->pptr());
	    current_->increaseCapacity(
	        computeNewCapacity_(current_->capacity(),
				    current_->size() + remaining*sizeof(CharT),
				    current_->maxCapacity())
	    );
	    resetStreamBufPtrs_();
	    available = (size_t)(this->epptr() - this->pptr());
	    if (!available) {
	      // Message buffer is too small to hold at least one char,
	      // so stop here
	      break;
	    }
	  }

	  size_t nToWrite = std::min(available, (size_t)remaining);
	  memcpy((void*)this->pptr(), (const void*)p, nToWrite*sizeof(CharT));
	  this->pbump(nToWrite);
	  p += nToWrite;
	  remaining -= nToWrite;
	  nWritten += nToWrite;
	}
	return nWritten;
      }

      virtual typename TraitsT::int_type overflow(
	  typename TraitsT::int_type c= TraitsT::eof()
      ) {
	if (!this->pptr()) {
	  // If the stream buffer called overflow(eof) without any message,
	  // it may be trying to allocate some space prior to writing a
	  // character, so obtain a new message for it to write to.
	  getNewMessage_();
	} else if (this->pptr() == this->epptr()) {
	  // Buffer is genuinely full, so try to increase its size.  Once
	  // we reach the maximum size, flush the message and obtain a new
	  // one.
	  current_->setEnd((char*)this->pptr());
	  size_t oldCapacity= current_->capacity();
	  size_t newCapacity= current_->increaseCapacity(oldCapacity*2);
	  if (newCapacity < sizeof(CharT)) {
	    // Buffer needs to be large enough to hold at least one char...
	    newCapacity= current_->increaseCapacity(sizeof(CharT));
	    if (newCapacity < sizeof(CharT)) {
	      // Maximum size of buffer is less than one char, abort...
	      return TraitsT::eof();
	    }
	  }
	  if (oldCapacity == newCapacity) {
	    sync();
	    getNewMessage_();
	  } else {
	      resetStreamBufPtrs_();
	  }
	}
	if (c != TraitsT::eof()) {
	  *this->pptr() = c;
	  this->pbump(1);
	}
	return c;
      }

      void getNewMessage_() {
	current_ = msgFactory_.get();
	current_->setLogLevel(logLevel_);
	current_->setDestination(destination_);
	resetStreamBufPtrs_();
      }

      void resetStreamBufPtrs_() {
	CharT* eos= (CharT*)current_->eos();
	if (sizeof(CharT) > 1) {
	  // Fix end pointer so it is guaranteed to lie within the buffer
	  eos =
	    (CharT*)(current_->begin() + (current_->capacity()/sizeof(CharT)));
	}
	this->setp((CharT*)current_->begin(), eos);
	this->pbump(current_->size()/sizeof(CharT));
      }

      size_t computeNewCapacity_(size_t current, size_t target,
				 size_t maxCapacity) {
	size_t newCapacity = current;
	while (newCapacity < target) {
	  if (newCapacity >= maxCapacity) {
	    return maxCapacity;
	  }
	  newCapacity *= 2;
	}
	return newCapacity;
      }
      
    private:
      LogMessageFactory& msgFactory_;
      LogMessageReceiver& msgReceiver_;
      const std::string& destination_;
      LogLevel logLevel_;
      LogMessage* current_;
    };

  }
}

#endif
