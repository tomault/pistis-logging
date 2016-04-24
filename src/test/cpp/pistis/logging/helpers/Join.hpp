#ifndef __PISTIS__LOGGING__HELPERS__JOIN_HPP__
#define __PISTIS__LOGGING__HELPERS__JOIN_HPP__

#include <sstream>
#include <string>

namespace pistis {
  namespace logging {

    template <typename Iterator>
    std::string join(Iterator begin, Iterator end,
		     const std::string& separator) {
      std::ostringstream tmp;
      for (auto i = begin; i != end; ++i) {
	if (i != begin) {
	  tmp << separator;
	}
	tmp << *i;
      }
      return tmp.str();
    }
    
  }
}
#endif
