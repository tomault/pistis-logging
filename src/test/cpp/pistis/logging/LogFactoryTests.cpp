#include <pistis/logging/LogFactory.hpp>
#include <gtest/gtest.h>

using namespace pistis::logging;

TEST(LogFactoryTests, GetInstance) {
  LogFactory* instance= LogFactory::getInstance();
  
  EXPECT_TRUE(instance != nullptr);
  EXPECT_EQ(instance, LogFactory::getInstance());
}

TEST(LogFactoryTests, GetLog) {
  const std::string DESTINATION("some.destination");
  Log* log= LogFactory::getLog(DESTINATION);
  
  ASSERT_TRUE(log != nullptr);
  EXPECT_EQ(log->destination(), DESTINATION);
  EXPECT_EQ(LogFactory::getLog(DESTINATION), log);
}

TEST(LogFactoryTests, SetLogLevelTest) {
  const std::string DESTINATION("some.destination");
  Log* log= LogFactory::getLog(DESTINATION);

  ASSERT_TRUE(log != nullptr);
  ASSERT_NE(log->logLevel(), LogLevel::WARN);
  LogFactory::setLogLevel(DESTINATION, LogLevel::WARN);
  EXPECT_EQ(log->logLevel(), LogLevel::WARN);
}
