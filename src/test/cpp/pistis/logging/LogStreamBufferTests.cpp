#include <pistis/logging/LogStreamBuffer.hpp>
#include <pistis/logging/SimpleLogMessageFactory.hpp>
#include <gtest/gtest.h>
#include <memory>

#include "helpers/TrackingLogMessageReceiver.hpp"

using namespace pistis::logging;

TEST(LogStreamBufferTests, Construct) {
  const std::string DESTINATION= "some.destination";
  SimpleLogMessageFactory msgFactory(16, 32);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStreamBuffer<char> buffer(msgFactory, msgReceiver, DESTINATION,
			       LogLevel::WARN);

  EXPECT_EQ(buffer.destination(), DESTINATION);
  EXPECT_EQ(buffer.logLevel(), LogLevel::WARN);
}

TEST(LogStreamBufferTests, WriteOneByOneNotOverflowing) {
  static const size_t INITIAL_CAPACITY= 16;
  static const size_t MAX_CAPACITY= 32;
  const std::string DESTINATION= "some.destination";
  const std::string MESSAGE= "abcdefghijklm";
  SimpleLogMessageFactory msgFactory(INITIAL_CAPACITY, MAX_CAPACITY);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStreamBuffer<char> buffer(msgFactory, msgReceiver, DESTINATION, LogLevel::WARN);

  for (auto i= MESSAGE.begin(); i != MESSAGE.end(); ++i) {
    EXPECT_EQ(buffer.sputc(*i), *i);
  }
  EXPECT_NE(buffer.pubsync(), -1);

  ASSERT_EQ(msgReceiver.messages().size(), 1);
  LogMessage* msg= msgReceiver.messages().front();
  std::string text(msg->begin(), msg->end());
  EXPECT_EQ(text, MESSAGE);
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::WARN);
  EXPECT_EQ(msg->capacity(), INITIAL_CAPACITY);
}

TEST(LogStreamBufferTests, WriteOneByOneIncreasingCapacity) {
  static const size_t INITIAL_CAPACITY= 16;
  static const size_t MAX_CAPACITY= 40;
  const std::string DESTINATION= "some.destination";
  const std::string MESSAGE= "abcdefghijklmnopqrstuvwxyz";
  SimpleLogMessageFactory msgFactory(INITIAL_CAPACITY, MAX_CAPACITY);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStreamBuffer<char> buffer(msgFactory, msgReceiver, DESTINATION,
			       LogLevel::WARN);

  for (auto i= MESSAGE.begin(); i != MESSAGE.end(); ++i) {
    EXPECT_EQ(buffer.sputc(*i), *i);
  }
  EXPECT_NE(buffer.pubsync(), -1);

  ASSERT_EQ(msgReceiver.messages().size(), 1);
  LogMessage* msg= msgReceiver.messages().front();
  std::string text(msg->begin(), msg->end());
  EXPECT_EQ(text, MESSAGE);
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::WARN);
  EXPECT_EQ(msg->capacity(), 2*INITIAL_CAPACITY);
}

TEST(LogStreamBufferTests, WriteOneByOneOverflowingMsg) {
  static const size_t INITIAL_CAPACITY= 16;
  static const size_t MAX_CAPACITY= 32;
  const std::string DESTINATION= "some.destination";
  const std::string MESSAGE= "abcdefghijklmnopqrstuvwxyz0123456789";
  SimpleLogMessageFactory msgFactory(INITIAL_CAPACITY, MAX_CAPACITY);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStreamBuffer<char> buffer(msgFactory, msgReceiver, DESTINATION,
			       LogLevel::WARN);

  for (auto i= MESSAGE.begin(); i != MESSAGE.end(); ++i) {
    EXPECT_EQ(buffer.sputc(*i), *i);
  }
  EXPECT_NE(buffer.pubsync(), -1);

  ASSERT_EQ(msgReceiver.messages().size(), 2);
  LogMessage* msg= msgReceiver.messages().front();
  std::string text(msg->begin(), msg->end());
  EXPECT_EQ(text, MESSAGE.substr(0,32));
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::WARN);
  EXPECT_EQ(msg->capacity(), MAX_CAPACITY);

  msg= msgReceiver.messages()[1];
  text.assign(msg->begin(), msg->end());
  EXPECT_EQ(text, MESSAGE.substr(32));
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::WARN);
  EXPECT_EQ(msg->capacity(), INITIAL_CAPACITY);
}

TEST(LogStreamBufferTests, WriteBlockNotOverflowing) {
  static const size_t INITIAL_CAPACITY= 16;
  static const size_t MAX_CAPACITY= 32;
  const std::string DESTINATION= "some.destination";
  const std::string MESSAGE= "abcdefghijklm";
  SimpleLogMessageFactory msgFactory(INITIAL_CAPACITY, MAX_CAPACITY);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStreamBuffer<char> buffer(msgFactory, msgReceiver, DESTINATION, LogLevel::WARN);

  EXPECT_EQ(buffer.sputn(MESSAGE.c_str(), MESSAGE.size()),
		    MESSAGE.size());
  EXPECT_NE(buffer.pubsync(), -1);

  ASSERT_EQ(msgReceiver.messages().size(), 1);
  LogMessage* msg= msgReceiver.messages().front();
  std::string text(msg->begin(), msg->end());
  EXPECT_EQ(text, MESSAGE);
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::WARN);
  EXPECT_EQ(msg->capacity(), INITIAL_CAPACITY);
}

TEST(LogStreamBufferTests, WriteBlockIncreasingCapacity) {
  static const size_t INITIAL_CAPACITY= 16;
  static const size_t MAX_CAPACITY= 40;
  const std::string DESTINATION= "some.destination";
  const std::string MESSAGE= "abcdefghijklmnopqrstuvwxyz";
  SimpleLogMessageFactory msgFactory(INITIAL_CAPACITY, MAX_CAPACITY);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStreamBuffer<char> buffer(msgFactory, msgReceiver, DESTINATION,
			       LogLevel::WARN);

  EXPECT_EQ(buffer.sputn(MESSAGE.c_str(), MESSAGE.size()),
		    MESSAGE.size());
  EXPECT_NE(buffer.pubsync(), -1);

  ASSERT_EQ(msgReceiver.messages().size(), 1);
  LogMessage* msg= msgReceiver.messages().front();
  std::string text(msg->begin(), msg->end());
  EXPECT_EQ(text, MESSAGE);
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::WARN);
  EXPECT_EQ(msg->capacity(), 2*INITIAL_CAPACITY);
}

TEST(LogStreamBufferTests, WriteBlockOverflowingMsg) {
  static const size_t INITIAL_CAPACITY= 16;
  static const size_t MAX_CAPACITY= 32;
  const std::string DESTINATION= "some.destination";
  const std::string MESSAGE= "abcdefghijklmnopqrstuvwxyz0123456789";
  SimpleLogMessageFactory msgFactory(INITIAL_CAPACITY, MAX_CAPACITY);
  TrackingLogMessageReceiver msgReceiver(&msgFactory);
  LogStreamBuffer<char> buffer(msgFactory, msgReceiver, DESTINATION,
			       LogLevel::WARN);

  EXPECT_EQ(buffer.sputn(MESSAGE.c_str(), MESSAGE.size()),
		    MESSAGE.size());
  EXPECT_NE(buffer.pubsync(), -1);

  ASSERT_EQ(msgReceiver.messages().size(), 2);
  LogMessage* msg= msgReceiver.messages().front();
  std::string text(msg->begin(), msg->end());
  EXPECT_EQ(text, MESSAGE.substr(0,32));
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::WARN);
  EXPECT_EQ(msg->capacity(), MAX_CAPACITY);

  msg= msgReceiver.messages()[1];
  text.assign(msg->begin(), msg->end());
  EXPECT_EQ(text, MESSAGE.substr(32));
  EXPECT_EQ(msg->destination(), DESTINATION);
  EXPECT_EQ(msg->logLevel(), LogLevel::WARN);
  EXPECT_EQ(msg->capacity(), INITIAL_CAPACITY);
}
