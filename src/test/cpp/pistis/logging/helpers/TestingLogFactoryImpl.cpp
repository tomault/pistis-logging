#include "TestingLogFactoryImpl.hpp"

using namespace pistis::logging;

TestingLogFactoryImpl::TestingLogFactoryImpl():
    logs_(),
    msgFactory_(new TrackingLogMessageFactory(initialMessageSize(),
					      maxMessageSize())),
    msgReceiver_(new TestingLogMessageReceiver(msgFactory_.get())),
    numGetLogCalls_(0), numSetLogLevelCalls_(0) {
  // Intentionally left blank
}

TestingLogFactoryImpl::~TestingLogFactoryImpl() {
  for (auto i= logs_.begin(); i != logs_.end(); ++i) {
    delete i->second;
  }
}

TestingLog* TestingLogFactoryImpl::getLog(const std::string& destination) {
  auto i= logs_.find(destination);

  ++numGetLogCalls_;
  if (i != logs_.end()) {
    return i->second;
  } else {
    TestingLog* l= new TestingLog(msgFactory_.get(), msgReceiver_.get(),
				  destination, defaultLogLevel());
    logs_.insert(std::make_pair(destination, l));
    return l;
  }
}

void TestingLogFactoryImpl::setLogLevel(const std::string& destination,
					LogLevel logLevel) {
  TestingLog* l= getLog(destination);
  l->setLogLevel(logLevel);
  ++numSetLogLevelCalls_;
}

LogFactoryImpl* pistis::logging::createLogFactoryImpl() {
  return new TestingLogFactoryImpl;
}
