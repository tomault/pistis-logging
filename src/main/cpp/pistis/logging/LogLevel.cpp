#include "LogLevel.hpp"
#include <unordered_map>
#include <vector>

using namespace pistis::logging;

namespace {
  class LevelToNameMap {
  public:
    LevelToNameMap():
        names_{ "", "TRACE", "DEBUG", "INFO", "WARN", "ERROR" } {
      // Intentionally left blank
    }
    
    const std::string& operator[](LogLevel level) const {
      return names_[(uint32_t)level];
    }

  private:
    std::vector<std::string> names_;
  };

  class NameToLevelMap {
  private:
    typedef std::unordered_map<std::string, LogLevel>::value_type Item;
    
  public:
    NameToLevelMap():
        levels_{ Item{ "TRACE", LogLevel::TRACE },
	         Item{ "DEBUG", LogLevel::DEBUG },
		 Item{ "INFO", LogLevel::INFO },
		 Item{ "WARN", LogLevel::WARN },
		 Item{ "ERROR", LogLevel::ERROR } } {
      // Intentionally left blank
    }

    std::pair<bool, LogLevel> operator[](const std::string& name) const {
      auto i = levels_.find(name);
      if (i == levels_.end()) {
	return std::make_pair(false, LogLevel::ERROR);
      } else {
	return std::make_pair(true, i->second);
      }
    }

  private:
    std::unordered_map<std::string, LogLevel> levels_;
  };

}

std::pair<bool, LogLevel> pistis::logging::parseLogLevel(
    const std::string& text
) {
  static const NameToLevelMap LEVEL_FOR_NAME;
  return LEVEL_FOR_NAME[text];
}

const std::string& pistis::logging::toString(LogLevel level) {
  static const LevelToNameMap NAME_FOR_LEVEL;
  return NAME_FOR_LEVEL[level];
}
