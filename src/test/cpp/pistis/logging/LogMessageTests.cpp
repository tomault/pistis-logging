#include <pistis/logging/LogMessage.hpp>
#include <gtest/gtest.h>

using namespace pistis::logging;

TEST(LogMessageTests, ConstructAtMaxCapacity) {
  static const size_t CAPACITY= 1024;
  LogMessage msg(CAPACITY);

  EXPECT_TRUE(msg.empty());
  EXPECT_FALSE(msg.full());
  EXPECT_TRUE(msg.atMaxCapacity());
  EXPECT_EQ(msg.size(), 0);
  EXPECT_EQ(msg.available(), CAPACITY);
  EXPECT_EQ(msg.capacity(), CAPACITY);
  EXPECT_EQ(msg.maxCapacity(), CAPACITY);
  EXPECT_EQ(msg.destination(), "");
  EXPECT_EQ(msg.logLevel(), LogLevel());
  EXPECT_NE(msg.begin(), (char*)0);
  EXPECT_EQ(msg.begin(), msg.end());
  EXPECT_EQ(msg.eos() - msg.begin(), CAPACITY);
}

TEST(LogMessageTests, ConstructWithInitialAndMaxCapacity) {
  static const size_t INITIAL_CAPACITY= 1024;
  static const size_t MAX_CAPACITY= 4096;
  LogMessage msg(INITIAL_CAPACITY, MAX_CAPACITY);

  EXPECT_TRUE(msg.empty());
  EXPECT_FALSE(msg.full());
  EXPECT_FALSE(msg.atMaxCapacity());
  EXPECT_EQ(msg.size(), 0);
  EXPECT_EQ(msg.available(), INITIAL_CAPACITY);
  EXPECT_EQ(msg.capacity(), INITIAL_CAPACITY);
  EXPECT_EQ(msg.maxCapacity(), MAX_CAPACITY);
  EXPECT_EQ(msg.destination(), "");
  EXPECT_EQ(msg.logLevel(), LogLevel());
  EXPECT_NE(msg.begin(), (char*)0);
  EXPECT_EQ(msg.begin(), msg.end());
  EXPECT_EQ(msg.eos() - msg.begin(), INITIAL_CAPACITY);
}

TEST(LogMessageTests, SetEnd) {
  static const size_t CAPACITY= 1024;
  static const size_t IN_USE= CAPACITY/4;
  LogMessage msg(CAPACITY);
  char* p= msg.begin() + IN_USE;

  msg.setEnd(p);
  EXPECT_FALSE(msg.empty());
  EXPECT_FALSE(msg.full());
  EXPECT_EQ(msg.size(), IN_USE);
  EXPECT_EQ(msg.available(), CAPACITY - IN_USE);
  EXPECT_EQ(msg.capacity(), CAPACITY);
  EXPECT_EQ(msg.maxCapacity(), CAPACITY);
  EXPECT_EQ(msg.end(), p);
  EXPECT_EQ(msg.eos(), msg.begin() + CAPACITY);

  msg.setEnd(msg.begin() + CAPACITY);
  EXPECT_FALSE(msg.empty());
  EXPECT_TRUE(msg.full());
  EXPECT_EQ(msg.size(), CAPACITY);
  EXPECT_EQ(msg.available(), 0);
  EXPECT_EQ(msg.end(), msg.eos());
}

TEST(LogMessageTests, SetLogLevel) {
  LogMessage msg(1024);

  msg.setLogLevel(LogLevel::INFO);
  EXPECT_EQ(msg.logLevel(), LogLevel::INFO);
}

TEST(LogMessageTests, SetDestination) {
  static const std::string DESTINATION= "some.destination";
  LogMessage msg(1024);

  msg.setDestination(DESTINATION);
  EXPECT_EQ(msg.destination(), DESTINATION);
}

TEST(LogMessageTests, IncreaseCapacity) {
  static const size_t INITIAL_CAPACITY= 1024;
  static const size_t MAX_CAPACITY= 4096;
  static const size_t MID_CAPACITY= 2048; // Part-way between initial and max
  static const size_t BEYOND_MAX_CAPACITY= 6000;
  static const char DATA[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  static const size_t IN_USE= sizeof(DATA-1);
  LogMessage msg(INITIAL_CAPACITY, MAX_CAPACITY);

  msg.setEnd(msg.begin() + IN_USE);
  memcpy(msg.begin(), DATA, IN_USE);

  ASSERT_EQ(msg.capacity(), INITIAL_CAPACITY);
  ASSERT_EQ(msg.maxCapacity(), MAX_CAPACITY);
  ASSERT_EQ(msg.size(), IN_USE);
  ASSERT_EQ(msg.end() - msg.begin(), IN_USE);

  // Increase size to midpoint should be ok
  size_t newCapacity= msg.increaseCapacity(MID_CAPACITY);

  EXPECT_EQ(newCapacity, MID_CAPACITY);
  EXPECT_EQ(msg.capacity(), MID_CAPACITY);
  EXPECT_EQ(msg.maxCapacity(), MAX_CAPACITY);
  EXPECT_FALSE(msg.atMaxCapacity());
  EXPECT_EQ(msg.size(), IN_USE);
  EXPECT_EQ(msg.end() - msg.begin(), IN_USE);
  EXPECT_FALSE(memcmp(msg.begin(), DATA, IN_USE));
  
  // Trying to reduce the size of the message should fail, leaving the
  // message intact
  char* oldBegin= msg.begin();
  char* oldEnd= msg.end();
  char* oldEos= msg.eos();
  newCapacity= msg.increaseCapacity(INITIAL_CAPACITY);

  EXPECT_EQ(newCapacity, MID_CAPACITY);
  EXPECT_EQ(msg.capacity(), MID_CAPACITY);
  EXPECT_EQ(msg.maxCapacity(), MAX_CAPACITY);
  EXPECT_EQ(msg.size(), IN_USE);
  EXPECT_EQ(msg.begin(), oldBegin);
  EXPECT_EQ(msg.end(), oldEnd);
  EXPECT_EQ(msg.eos(), oldEos);

  // Increase capacity past the maximum.  Eliminate data in the buffer to
  // test the branch in LogMessage::_increaseBufferSize() that is taken
  // when the message is empty
  msg.setEnd(msg.begin());
  newCapacity= msg.increaseCapacity(BEYOND_MAX_CAPACITY);

  EXPECT_EQ(newCapacity, MAX_CAPACITY);
  EXPECT_EQ(msg.capacity(), MAX_CAPACITY);
  EXPECT_EQ(msg.maxCapacity(), MAX_CAPACITY);
  EXPECT_EQ(msg.size(), 0);
  EXPECT_EQ(msg.eos() - msg.begin(), MAX_CAPACITY);
}

TEST(LogMessageTests, Move) {
  static const size_t INITIAL_CAPACITY= 1024;
  static const size_t MAX_CAPACITY= 4096;
  static const size_t IN_USE= 256;
  static const std::string DESTINATION= "some.destination";
  LogMessage msg(INITIAL_CAPACITY, MAX_CAPACITY);
  msg.setEnd(msg.begin() + IN_USE);
  msg.setDestination(DESTINATION);
  msg.setLogLevel(LogLevel::ERROR);

  char* msgBegin= msg.begin();
  char* msgEnd= msg.end();
  char* msgEos= msg.eos();

  LogMessage moved(std::move(msg));
  EXPECT_FALSE(moved.empty());
  EXPECT_FALSE(moved.full());
  EXPECT_FALSE(moved.atMaxCapacity());
  EXPECT_EQ(moved.size(), IN_USE);
  EXPECT_EQ(moved.available(), INITIAL_CAPACITY - IN_USE);
  EXPECT_EQ(moved.capacity(), INITIAL_CAPACITY);
  EXPECT_EQ(moved.maxCapacity(), MAX_CAPACITY);
  EXPECT_EQ(moved.logLevel(), LogLevel::ERROR);
  EXPECT_EQ(moved.destination(), DESTINATION);
  EXPECT_EQ(moved.begin(), msgBegin);
  EXPECT_EQ(moved.end(), msgEnd);
  EXPECT_EQ(moved.eos(), msgEos);

  // LogLevel and destination are undefined after a move
  EXPECT_TRUE(msg.empty());
  EXPECT_EQ(msg.size(), 0);
  EXPECT_EQ(msg.capacity(), 0);
  EXPECT_EQ(msg.maxCapacity(), 0);
  EXPECT_EQ(msg.begin(), (char*)0);
  EXPECT_EQ(msg.end(), (char*)0);
  EXPECT_EQ(msg.eos(), (char*)0);
}

TEST(LogMessageTests, MoveAssignment) {
  static const size_t INITIAL_CAPACITY= 1024;
  static const size_t MID_CAPACITY= 2048;
  static const size_t MAX_CAPACITY= 4096;
  static const size_t IN_USE= 256;
  static const std::string DESTINATION= "some.destination";
  LogMessage msg(INITIAL_CAPACITY, MAX_CAPACITY);
  LogMessage moved(MID_CAPACITY);
  msg.setEnd(msg.begin() + IN_USE);
  msg.setDestination(DESTINATION);
  msg.setLogLevel(LogLevel::ERROR);

  char* msgBegin= msg.begin();
  char* msgEnd= msg.end();
  char* msgEos= msg.eos();

  moved= std::move(msg);
  EXPECT_FALSE(moved.empty());
  EXPECT_FALSE(moved.full());
  EXPECT_FALSE(moved.atMaxCapacity());
  EXPECT_EQ(moved.size(), IN_USE);
  EXPECT_EQ(moved.available(), INITIAL_CAPACITY - IN_USE);
  EXPECT_EQ(moved.capacity(), INITIAL_CAPACITY);
  EXPECT_EQ(moved.maxCapacity(), MAX_CAPACITY);
  EXPECT_EQ(moved.logLevel(), LogLevel::ERROR);
  EXPECT_EQ(moved.destination(), DESTINATION);
  EXPECT_EQ(moved.begin(), msgBegin);
  EXPECT_EQ(moved.end(), msgEnd);
  EXPECT_EQ(moved.eos(), msgEos);

  // LogLevel and destination are undefined after a move
  EXPECT_TRUE(msg.empty());
  EXPECT_EQ(msg.size(), 0);
  EXPECT_EQ(msg.capacity(), 0);
  EXPECT_EQ(msg.maxCapacity(), 0);
  EXPECT_EQ(msg.begin(), (char*)0);
  EXPECT_EQ(msg.end(), (char*)0);
  EXPECT_EQ(msg.eos(), (char*)0);
}
