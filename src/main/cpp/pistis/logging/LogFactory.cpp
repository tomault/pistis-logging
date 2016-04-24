#include "LogFactory.hpp"

using namespace pistis::logging;

LogFactory* LogFactory::theFactory_ = nullptr;
LogFactory::Destructor LogFactory::destructor_;
std::once_flag LogFactory::onceFlag_;

LogFactory::LogFactory(LogFactoryImpl* impl):
  impl_(impl) {
}

LogFactory::~LogFactory() {
  delete impl_;
}

void LogFactory::initFactory_() {
  theFactory_ = new LogFactory(createLogFactoryImpl());
  destructor_.factory = theFactory_;
}

LogFactory::Destructor::Destructor():
  factory(nullptr) {
}

LogFactory::Destructor::~Destructor() {
  delete factory;
}


