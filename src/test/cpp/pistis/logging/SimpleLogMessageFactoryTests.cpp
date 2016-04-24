#include <pistis/logging/SimpleLogMessageFactory.hpp>
#include <gtest/gtest.h>
#include <algorithm>
#include <set>
#include <vector>

#include "helpers/Join.hpp"
#include "helpers/LockingGate.hpp"
#include "helpers/LogMessageConsumer.hpp"
#include "helpers/LogMessageWaiter.hpp"
#include "helpers/ThreadSet.hpp"

using namespace pistis::logging;

TEST(SimpleLogMessageFactoryTests, GetAndReleaseTest) {
  static const size_t INITIAL_CAPACITY=128;
  static const size_t MAX_CAPACITY= 1024;
  SimpleLogMessageFactory factory(INITIAL_CAPACITY, MAX_CAPACITY);
  LogMessage* msg= factory.get();

  EXPECT_EQ(msg->capacity(), factory.initialMessageSize());
  EXPECT_EQ(msg->maxCapacity(), factory.maxMessageSize());
  EXPECT_TRUE(msg->empty());

  // No way to check if release was actually done except in valgrind...
  factory.release(msg);
}

TEST(SimpleLogMessageFactoryTests, SimultaneousGetTest) {
  static const size_t INITIAL_CAPACITY=128;
  static const size_t MAX_CAPACITY= 1024;
  static const std::chrono::system_clock::time_point DEADLINE=
    std::chrono::system_clock::now() + std::chrono::seconds(2);
  SimpleLogMessageFactory factory(INITIAL_CAPACITY, MAX_CAPACITY);
  LockingGate acquireGate;
  LockingGate releaseGate;
  LockingGate exitGate;
  ThreadSet<LogMessageConsumer> consumers(100, 1, &factory, &acquireGate, &releaseGate, &exitGate, DEADLINE);
  std::vector<std::string> errors;
  std::set<LogMessage*> messages;
  std::vector<LogMessage*> badCapacity;

  if (!acquireGate.waitForThreads(consumers.size(), DEADLINE)) {
    EXPECT_TRUE(false) << "Failed to get all threads to the acquire gate "
		       << "before the deadline";
    return;
  }
  acquireGate.open();
  if (!acquireGate.waitUntilClear(DEADLINE)) {
    EXPECT_TRUE(false) << "All threads did not clear the aquire gate before "
		       << "the deadline";
    return;
  }
  if (!releaseGate.waitForThreads(100, DEADLINE)) {
    EXPECT_TRUE(false) << "All threads did not reach the release gate before "
		       << "the deadline";
    return;
  }
  errors= consumers.scanForErrors();
  if (!errors.empty()) {
    std::ostringstream details;
    details << "While acquiring messages, the following errors occurred:\n  "
	    << join(errors.begin(), errors.end(), "  \n");
    EXPECT_TRUE(false) << details.str();
    return;
  }
  
  EXPECT_EQ(factory.numMessagesActive(), 100);
  // Check that each consumer received a unique message and that all
  // messages have the correct capacity
  std::for_each(consumers.begin(), consumers.end(),
		[&messages](LogMessageConsumer* c) {
		  messages.insert(c->message());
		}
  );
  ASSERT_EQ(messages.size(), 100);

  for (auto i= messages.begin(); i != messages.end(); ++i) {
    if (((*i)->capacity() != INITIAL_CAPACITY) ||
	((*i)->maxCapacity() != MAX_CAPACITY) ||
	!((*i)->empty())) {
      badCapacity.push_back(*i);
    }
  }
  if (!badCapacity.empty()) {
    std::ostringstream details;
    details << badCapacity.size()
	    << " threads received a message that did not have the correct "
	    << "capacity or was not empty:";
    std::for_each(badCapacity.begin(), badCapacity.end(),
		  [&details](const LogMessage* m) {
		    details << " " << m << "[" << m->capacity() << "/"
			    << m->maxCapacity() << "/" << m->size() << "]";
		  });
    EXPECT_TRUE(false) << details.str();
  }

  releaseGate.open();
  if (!releaseGate.waitUntilClear(DEADLINE)) {
    EXPECT_TRUE(false) << "All threads did not clear the release gate before "
		       << "the deadline";
    return;
  }
  if (!exitGate.waitForThreads(100, DEADLINE)) {
    EXPECT_TRUE(false) << "All threads did not reach the exit gate before the "
		       << "deadline";
    return;
  }
  exitGate.open();

  EXPECT_EQ(factory.numMessagesActive(), 0);

  errors= consumers.scanForErrors();
  if (!errors.empty()) {
    std::ostringstream details;
    details << "While acquiring messages, the following errors occurred:\n  "
	    << join(errors.begin(), errors.end(), "  \n");
    EXPECT_TRUE(false) << details.str();
    return;
  }
}

TEST(SimpleLogMessageFactoryTests, WaitTest) {
  static const size_t INITIAL_CAPACITY=128;
  static const size_t MAX_CAPACITY= 1024;
  static const std::chrono::system_clock::time_point DEADLINE=
    std::chrono::system_clock::now() + std::chrono::seconds(10);
  SimpleLogMessageFactory factory(INITIAL_CAPACITY, MAX_CAPACITY);
  LockingGate acquireGate;
  LockingGate releaseGate;
  LockingGate exitGate;
  LockingGate waitGate1;
  LockingGate waitGate2;
  LockingGate exitGate1;
  LockingGate exitGate2;
  ThreadSet<LogMessageConsumer> consumers(100, 1, &factory, &acquireGate, &releaseGate, &exitGate, DEADLINE);
  ThreadSet<LogMessageWaiter> waiters(10, 1, &factory, &waitGate1, &exitGate1, DEADLINE);
  std::vector<std::string> errors;
  std::vector<LogMessageWaiter*> notFound;

  /// Acquire 100 messages from the factory at once
  if (!acquireGate.waitForThreads(consumers.size(), DEADLINE)) {
    EXPECT_TRUE(false) << "Failed to get all threads to the acquire gate "
		       << "before the deadline";
    return;
  }
  acquireGate.open();

  if (!releaseGate.waitForThreads(100, DEADLINE)) {
    EXPECT_TRUE(false) << "All threads did not reach the release gate before "
		       << "the deadline";
    return;
  }
  errors= consumers.scanForErrors();
  if (!errors.empty()) {
    std::ostringstream details;
    details << "While acquiring messages, the following errors occurred:\n  "
	    << join(errors.begin(), errors.end(), "  \n");
    EXPECT_TRUE(false) << details.str();
    return;
  }
  
  EXPECT_EQ(factory.numMessagesActive(), 100);

  // Release the waiters
  waitGate1.open();

  // Test what happens when the deadline expires while waiting for messages
  // to return to the factory.
  const std::chrono::system_clock::time_point SHORT_DEADLINE=
    std::chrono::system_clock::now() + std::chrono::milliseconds(200);
  ThreadSet<LogMessageWaiter> expiringWaiters(10, 11, &factory, &waitGate2,
					      &exitGate2, SHORT_DEADLINE);
  if (!waitGate2.waitForThreads(10, DEADLINE)) {
    EXPECT_TRUE(false) << "Deadline expired while waiting for threads to "
		       << "arrive at the second wait gate";
    return;
  }
  waitGate2.open();
  exitGate2.waitUntilArrived(10, DEADLINE);

  exitGate2.open();  //  Let the threads in expiringWaiters exit
  errors= expiringWaiters.scanForErrors();
  if (!errors.empty()) {
    std::ostringstream details;
    details << "While waiting on a short deadline, the following errors occurred:\n  "
	    << join(errors.begin(), errors.end(), "  \n");
    EXPECT_TRUE(false) << details.str();
  }
  
  notFound.clear();
  std::for_each(expiringWaiters.begin(), expiringWaiters.end(),
		[&notFound](LogMessageWaiter* w) {
		  if (!w->deadlineExpiredWhileWaiting()) {
		    notFound.push_back(w);
		  }
		});
  if (!notFound.empty()) {
    std::ostringstream details;
    details << notFound.size()
	    << " waiters incorrectly notified the caller that all messages "
	    << "returned to the factory";
    EXPECT_TRUE(false) << details.str();
  }

  // Return messages to the pool and verify that the threads in "waiters"
  // all stopped waiting and received true from "waitUntilAllReturned"
  releaseGate.open();
  exitGate.waitUntilArrived(100, DEADLINE);
  exitGate.open(); // Let the consumer threads exit
  
  exitGate1.waitUntilArrived(10, DEADLINE);

  errors= waiters.scanForErrors();
  if (!errors.empty()) {
    std::ostringstream details;
    details << "While waiting for messages to return to the factory, the "
	    << "following errors occurred:\n  "
	    << join(errors.begin(), errors.end(), "  \n");
    EXPECT_TRUE(false) << details.str();
  }
  notFound.clear();
  std::for_each(waiters.begin(), waiters.end(),
		[&notFound](LogMessageWaiter* w) {
		  if (w->deadlineExpiredWhileWaiting()) {
		    notFound.push_back(w);
		  }
		});
  if (!notFound.empty()) {
    std::ostringstream details;
    details << notFound.size() << " waiters were incorrectly notified that "
	    << "the deadline expired while waiting for messages to return to "
	    << "the factory";
    EXPECT_TRUE(false) << details.str();
  }

  exitGate1.open();
}

