#include <pistis/logging/SimpleLogMessageFactory.hpp>
#include <gtest/gtest.h>

#include "helpers/TestingLog.hpp"
#include "helpers/TrackingLogMessageReceiver.hpp"

using namespace pistis::logging;

TEST(LogTests, ConstructTest) {
  const std::string DESTINATION("some.destination");
  SimpleLogMessageFactory msgFactory(256, 256);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  TestingLog log(&msgFactory, &msgReceiver, DESTINATION, LogLevel::INFO);

  EXPECT_EQ(log.destination(), DESTINATION);
  EXPECT_EQ(log.logLevel(), LogLevel::INFO);
}

TEST(LogTests, IsEnabledTest) {
  const std::string DESTINATION("some.destination");
  SimpleLogMessageFactory msgFactory(256, 256);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  TestingLog log(&msgFactory, &msgReceiver, DESTINATION, LogLevel::INFO);

  EXPECT_FALSE(log.isEnabled(LogLevel::TRACE));
  EXPECT_FALSE(log.isEnabled(LogLevel::DEBUG));
  EXPECT_TRUE(log.isEnabled(LogLevel::INFO));
  EXPECT_TRUE(log.isEnabled(LogLevel::WARN));
  EXPECT_TRUE(log.isEnabled(LogLevel::ERROR));
}

TEST(LogTests, LogTest) {
  const std::string DESTINATION("some.destination");
  SimpleLogMessageFactory msgFactory(256, 256);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  TestingLog log(&msgFactory, &msgReceiver, DESTINATION, LogLevel::INFO);
  Log& l= log;

  l.trace() << "This is the trace level";
  log.debug() << "This is the debug level";
  log.info() << "This is the info level";
  log.warn() << "This is the warn level";
  log.error() << "This is the error level";

  ASSERT_EQ(msgReceiver.messages().size(), 3);
  LogMessage* msg= msgReceiver.messages()[0];
  EXPECT_EQ(std::string(msg->begin(), msg->end()), "This is the info level");
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::INFO);

  msg= msgReceiver.messages()[1];
  EXPECT_EQ(std::string(msg->begin(), msg->end()), "This is the warn level");
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::WARN);

  msg= msgReceiver.messages()[2];
  EXPECT_EQ(std::string(msg->begin(), msg->end()), "This is the error level");
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::ERROR);
}
