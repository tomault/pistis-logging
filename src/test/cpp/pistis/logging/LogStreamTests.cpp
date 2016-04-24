#include <pistis/logging/LogStream.hpp>
#include <pistis/logging/SimpleLogMessageFactory.hpp>
#include <gtest/gtest.h>
#include <iomanip>

#include "helpers/TrackingLogMessageReceiver.hpp"

using namespace pistis::logging;

TEST(LogStreamTests, ConstructTest) {
  const std::string DESTINATION= "some.destination";
  const std::string MESSAGE= "abcdefghijklm";
  SimpleLogMessageFactory msgFactory(256, 256);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStream<char> s(msgFactory, msgReceiver, DESTINATION, LogLevel::INFO,
		    true);

  EXPECT_EQ(s.destination(), DESTINATION);
  EXPECT_EQ(s.logLevel(), LogLevel::INFO);
  EXPECT_TRUE(s.enabled());
}

TEST(LogStreamTests, WriteTest) {
  const std::string DESTINATION= "some.destination";
  const std::string TRUTH= "This is a number in base-16: abcdef\n";
  SimpleLogMessageFactory msgFactory(256, 256);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStream<char> s(msgFactory, msgReceiver, DESTINATION, LogLevel::INFO, true);

  s << "This is a number in base-" << 16 << ": " << std::hex << 0xABCDEF
    << std::endl;

  ASSERT_EQ(msgReceiver.messages().size(), 1);
  LogMessage* msg= msgReceiver.messages().front();
  EXPECT_EQ(std::string(msg->begin(), msg->end()), TRUTH);
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::INFO);
}

TEST(LogStreamTests, WriteRawTest) {
  const std::string DESTINATION= "some.destination";
  const std::string TRUTH= "This is a number in base-16: abcdef";
  SimpleLogMessageFactory msgFactory(256, 256);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStream<char> s(msgFactory, msgReceiver, DESTINATION, LogLevel::INFO, true);

  s.write(TRUTH.c_str(), TRUTH.size());
  s.flush();
  ASSERT_EQ(msgReceiver.messages().size(), 1);
  LogMessage* msg= msgReceiver.messages().front();
  EXPECT_EQ(std::string(msg->begin(), msg->end()), TRUTH);
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::INFO);
}

TEST(LogStreamTests, FlushOnDestroyTest) {
  const std::string DESTINATION= "some.destination";
  const std::string TRUTH= "This is a number in base-16: abcdef";
  SimpleLogMessageFactory msgFactory(256, 256);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);

  {
    LogStream<char> s(msgFactory, msgReceiver, DESTINATION, LogLevel::INFO, true);

    s << "This is a number in base-" << 16 << ": " << std::hex << 0xABCDEF;
  }

  ASSERT_EQ(msgReceiver.messages().size(), 1);
  LogMessage* msg= msgReceiver.messages().front();
  EXPECT_EQ(std::string(msg->begin(), msg->end()), TRUTH);
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::INFO);
}

TEST(LogStreamTests, DisabledTest) {
  const std::string DESTINATION= "some.destination";
  const std::string MORE_DATA= "This is more data";
  SimpleLogMessageFactory msgFactory(256, 256);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStream<char> s(msgFactory, msgReceiver, DESTINATION, LogLevel::INFO, false);

  s << "This is a number in base-" << 16 << ": " << std::hex << 0xABCDEF
    << std::endl;
  s.write(MORE_DATA.c_str(), MORE_DATA.size());
  s.flush();

  EXPECT_EQ(msgReceiver.messages().size(), 0);
}

TEST(LogStreamTests, MoveTest) {
  const std::string DESTINATION= "some.destination";
  const std::string TRUTH= "This is a number in base-16: abcdef\n";
  SimpleLogMessageFactory msgFactory(256, 256);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStream<char> s(msgFactory, msgReceiver, DESTINATION, LogLevel::INFO, true);
  LogStream<char> moved(std::move(s));

  // Note: s is no longer valid after a move, and using it will probably
  //       cause segfaults.
  moved << "This is a number in base-" << 16 << ": " << std::hex << 0xABCDEF
	<< std::endl;

  ASSERT_EQ(msgReceiver.messages().size(), 1);
  LogMessage* msg= msgReceiver.messages().front();
  EXPECT_EQ(std::string(msg->begin(), msg->end()), TRUTH);
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::INFO);
}

