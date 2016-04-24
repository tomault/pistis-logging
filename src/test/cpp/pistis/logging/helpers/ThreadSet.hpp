#ifndef __PISTIS__LOGGING__HELPERS__THREADSET_HPP__
#define __PISTIS__LOGGING__HELPERS__THREADSET_HPP__

#include <sstream>
#include <string>
#include <vector>
#include <stddef.h>

namespace pistis {
  namespace logging {

    template <typename T>
    class ThreadSet {
    public:
      typedef typename std::vector<T*>::const_iterator ConstIterator;

    public:
      template <typename... ArgsT>
      ThreadSet(size_t n, size_t startId, ArgsT&&... args):
	  threads_() {
	threads_.reserve(n);
	for (auto i=0;i<n;++i) {
	  threads_.push_back(
	      new T(indexToString_(startId+i), std::forward<ArgsT>(args)...)
	  );
	}
      }
  
      ~ThreadSet() {
	for (auto i= threads_.begin(); i != threads_.end(); ++i) {
	  delete *i;
	}
      }

      size_t size() const { return threads_.size(); }
      ConstIterator begin() const { return threads_.begin(); }
      ConstIterator end() const { return threads_.end(); }
      T* operator[](size_t n) const { return threads_[n]; }

      std::vector<std::string> scanForErrors() const {
	std::vector<std::string> errors;
	for (auto i= begin(); i != end(); ++i) {
	  if (!(*i)->errors().empty()) {
	    errors.push_back((*i)->errors());
	  }
	}
	return std::move(errors);
      }

    private:
      std::vector<T*> threads_;

      static std::string indexToString_(size_t n) {
	std::ostringstream tmp;
	tmp << n;
	return tmp.str();
      }
    };

  }
}
#endif

