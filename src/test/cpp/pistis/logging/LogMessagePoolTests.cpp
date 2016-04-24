#include <gtest/gtest.h>
#include <algorithm>
#include <iterator>
#include <set>
#include <vector>

#include "helpers/Join.hpp"
#include "helpers/LockingGate.hpp"
#include "helpers/LogMessageConsumer.hpp"
#include "helpers/LogMessageWaiter.hpp"
#include "helpers/TestingLogMessagePool.hpp"
#include "helpers/ThreadSet.hpp"

using namespace pistis::logging;

namespace {
  template <typename ContainerT>
  class PlainInsertIterator :
      public std::iterator<std::output_iterator_tag, void, void, void, void> {
  public:
    PlainInsertIterator(ContainerT& container): container_(container) { }

    PlainInsertIterator& operator=(const typename ContainerT::value_type& v) {
      container_.insert(v);
      return *this;
    }
    PlainInsertIterator& operator=(typename ContainerT::value_type&& v) {
      container_.insert(std::move(v));
      return *this;
    }
    PlainInsertIterator& operator*() { return *this; }
    PlainInsertIterator& operator++() { return *this; }
    PlainInsertIterator& operator++(int) { return *this; }

  private:
    ContainerT& container_;
  };

  template <typename ContainerT>
  PlainInsertIterator<ContainerT> plainInserter(ContainerT& c) {
    return PlainInsertIterator<ContainerT>(c);
  }
}

TEST(LogMessagePoolTests, GetAndRelease) {
  static const size_t INITIAL_CAPACITY=128;
  static const size_t MAX_CAPACITY= 1024;
  static const size_t MAX_RETURNED_MESSAGE_SIZE= 256;
  static const size_t INITIAL_POOL_SIZE= 4;
  static const size_t MAX_POOL_SIZE= 8;
  TestingLogMessagePool factory(INITIAL_CAPACITY, MAX_CAPACITY,
				MAX_RETURNED_MESSAGE_SIZE, INITIAL_POOL_SIZE,
				MAX_POOL_SIZE);
  std::vector<LogMessage*> messagesInPool;
  std::vector<LogMessage*> newMessagesInPool;
  LogMessage* msg;

  messagesInPool.reserve(INITIAL_POOL_SIZE);
  factory.getMessagesInPool(std::back_inserter(messagesInPool));
  EXPECT_EQ(messagesInPool.size(), INITIAL_POOL_SIZE);

  msg= factory.get();
  EXPECT_EQ(msg->capacity(), factory.initialMessageSize());
  EXPECT_EQ(msg->maxCapacity(), factory.maxMessageSize());
  EXPECT_TRUE(msg->empty());

  // Verify the message came from the pool
  EXPECT_EQ(factory.numMessagesInPool(), INITIAL_POOL_SIZE-1);
  EXPECT_TRUE(std::find(messagesInPool.begin(), messagesInPool.end(), msg) != messagesInPool.end());

  // Verify the message returned to the pool
  factory.release(msg);
  EXPECT_EQ(factory.numMessagesInPool(), INITIAL_POOL_SIZE);
  
  factory.getMessagesInPool(std::back_inserter(newMessagesInPool));
  std::sort(messagesInPool.begin(), messagesInPool.end());
  std::sort(newMessagesInPool.begin(), newMessagesInPool.end());
  if (messagesInPool != newMessagesInPool) {
    std::ostringstream details;
    details << "messagesInPool != newMessagesInPool\n  messagesInPool = [ "
	    << join(messagesInPool.begin(), messagesInPool.end(), ", ")
	    << " ]\n  newMessagesInPool = [ "
	    << join(newMessagesInPool.begin(), newMessagesInPool.end(), ", ")
	    << " ]";
    EXPECT_TRUE(false) << details.str();
  }
}

TEST(LogMessagePoolTests, ExhaustPool) {
  static const size_t INITIAL_CAPACITY=128;
  static const size_t MAX_CAPACITY= 1024;
  static const size_t MAX_RETURNED_MESSAGE_SIZE= 256;
  static const size_t INITIAL_POOL_SIZE= 4;
  static const size_t MAX_POOL_SIZE= 8;
  TestingLogMessagePool factory(INITIAL_CAPACITY, MAX_CAPACITY,
				MAX_RETURNED_MESSAGE_SIZE, INITIAL_POOL_SIZE,
				MAX_POOL_SIZE);
  std::set<LogMessage*> acquiredMessages;
  std::vector<LogMessage*> messagesInPool;
  size_t numFromPool;

  messagesInPool.reserve(INITIAL_POOL_SIZE);
  factory.getMessagesInPool(std::back_inserter(messagesInPool));
  ASSERT_EQ(messagesInPool.size(), 4);
  
  
  for (int i=0;i<MAX_POOL_SIZE+1;++i) {
    LogMessage* msg= factory.get();
    EXPECT_EQ(msg->capacity(), INITIAL_CAPACITY);
    EXPECT_EQ(msg->maxCapacity(), MAX_CAPACITY);
    EXPECT_TRUE(msg->empty());
    acquiredMessages.insert(msg);
  }
  // Verify we received no duplicate messages and four of them came from
  // the pool.  Note we use a O(M*N) algorithm, which is ok because our
  // sizes are guaranted to be small, but don't do that in production.
  ASSERT_EQ(acquiredMessages.size(), MAX_POOL_SIZE+1);
  numFromPool= 0;
  for (auto i= acquiredMessages.begin(); i != acquiredMessages.end(); ++i) {
    if (std::find(messagesInPool.begin(), messagesInPool.end(), *i) != messagesInPool.end()) {
      ++numFromPool;
    }
  }
  EXPECT_EQ(numFromPool, messagesInPool.size());
  EXPECT_EQ(factory.numMessagesInPool(), 0);

  // Return the messages to the pool and verify MAX_POOL_SIZE of them made
  // it into the pool.  No way to know if the remaining messages were actually
  // destroyed except to run through Valgrind.
  std::for_each(acquiredMessages.begin(), acquiredMessages.end(),
		[&factory](LogMessage* m) { factory.release(m); });
  messagesInPool.clear();
  messagesInPool.reserve(MAX_POOL_SIZE);
  factory.getMessagesInPool(std::back_inserter(messagesInPool));
  EXPECT_EQ(messagesInPool.size(), MAX_POOL_SIZE);
  for (auto i= messagesInPool.begin(); i != messagesInPool.end(); ++i) {
    EXPECT_TRUE(acquiredMessages.find(*i) != acquiredMessages.end());
  }
}

TEST(LogMessagePoolTests, ReturnTooLargeMessage) {
  static const size_t INITIAL_CAPACITY=128;
  static const size_t MAX_CAPACITY= 1024;
  static const size_t MAX_RETURNED_MESSAGE_SIZE= 256;
  static const size_t INITIAL_POOL_SIZE= 4;
  static const size_t MAX_POOL_SIZE= 8;
  TestingLogMessagePool factory(INITIAL_CAPACITY, MAX_CAPACITY,
				MAX_RETURNED_MESSAGE_SIZE, INITIAL_POOL_SIZE,
				MAX_POOL_SIZE);
  std::vector<LogMessage*> messagesInPool;
  std::vector<LogMessage*> newMessagesInPool;
  LogMessage* msg;

  messagesInPool.reserve(INITIAL_POOL_SIZE);
  factory.getMessagesInPool(std::back_inserter(messagesInPool));
  EXPECT_EQ(messagesInPool.size(), INITIAL_POOL_SIZE);

  msg= factory.get();
  EXPECT_EQ(msg->capacity(), factory.initialMessageSize());
  EXPECT_EQ(msg->maxCapacity(), factory.maxMessageSize());
  EXPECT_TRUE(msg->empty());

  // Verify the message came from the pool
  EXPECT_EQ(factory.numMessagesInPool(), INITIAL_POOL_SIZE-1);
  EXPECT_TRUE(std::find(messagesInPool.begin(), messagesInPool.end(), msg) != messagesInPool.end());

  // Make the message too big to be returned to the pool
  msg->increaseCapacity(MAX_RETURNED_MESSAGE_SIZE+16);
  ASSERT_GT(msg->capacity(), MAX_RETURNED_MESSAGE_SIZE);
  
  // Verify the message destroyed upon release and not put back in the pool
  factory.release(msg);
  EXPECT_EQ(factory.numMessagesInPool(), INITIAL_POOL_SIZE-1);
}

TEST(LogMessagePoolTests, ReleaseNonEmptyMessage) {
  static const size_t INITIAL_CAPACITY=128;
  static const size_t MAX_CAPACITY= 1024;
  static const size_t MAX_RETURNED_MESSAGE_SIZE= 256;
  static const size_t INITIAL_POOL_SIZE= 4;
  static const size_t MAX_POOL_SIZE= 8;
  TestingLogMessagePool factory(INITIAL_CAPACITY, MAX_CAPACITY,
				MAX_RETURNED_MESSAGE_SIZE, INITIAL_POOL_SIZE,
				MAX_POOL_SIZE);
  std::vector<LogMessage*> messages;
  LogMessage* msg;

  msg= factory.get();
  EXPECT_EQ(msg->capacity(), factory.initialMessageSize());
  EXPECT_EQ(msg->maxCapacity(), factory.maxMessageSize());
  EXPECT_TRUE(msg->empty());
  ASSERT_EQ(factory.numMessagesInPool(), INITIAL_POOL_SIZE-1);

  // Make the message not empty and return it to the pool
  msg->setEnd(msg->begin() + msg->capacity()/2);
  ASSERT_FALSE(msg->empty());
  factory.release(msg);
  ASSERT_EQ(factory.numMessagesInPool(), INITIAL_POOL_SIZE);

  // Now get all messages in the pool, verify that one of them is msg,
  // and that all messages (including msg) are empty after leaving the pool
  for (int i=0;i<INITIAL_POOL_SIZE;++i) {
    messages.push_back(factory.get());
    EXPECT_TRUE(messages.back()->empty());
  }
  EXPECT_EQ(factory.numMessagesInPool(), 0);
  EXPECT_TRUE(std::find(messages.begin(), messages.end(), msg) != messages.end());

  // Put everything back
  std::for_each(messages.begin(), messages.end(),
		[&factory](LogMessage* m) { factory.release(m); });
}

TEST(LogMessagePoolTests, SimultaneousGet) {
  static const size_t INITIAL_CAPACITY=128;
  static const size_t MAX_CAPACITY= 1024;
  static const size_t MAX_RETURNED_MESSAGE_SIZE= 256;
  static const size_t INITIAL_POOL_SIZE= 4;
  static const size_t MAX_POOL_SIZE= 90;
  static const std::chrono::system_clock::time_point DEADLINE=
    std::chrono::system_clock::now() + std::chrono::seconds(2);
  TestingLogMessagePool factory(INITIAL_CAPACITY, MAX_CAPACITY,
				MAX_RETURNED_MESSAGE_SIZE, INITIAL_POOL_SIZE,
				MAX_POOL_SIZE);
  LockingGate acquireGate;
  LockingGate releaseGate;
  LockingGate exitGate;
  ThreadSet<LogMessageConsumer> consumers(100, 1, &factory, &acquireGate, &releaseGate, &exitGate, DEADLINE);
  std::vector<std::string> errors;
  std::set<LogMessage*> messages;
  std::vector<LogMessage*> badCapacity;
  std::set<LogMessage*> messagesInPool;
  size_t numFromPool;

  factory.getMessagesInPool(plainInserter(messagesInPool));
  ASSERT_EQ(messagesInPool.size(), INITIAL_POOL_SIZE);
    
  // Have 100 threads acquire messages at once
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
  // Check that each consumer received a unique message, all messages have
  // the correct capacity and at least INITIAL_POOL_SIZE of them came from
  // the pool
  std::for_each(consumers.begin(), consumers.end(),
		[&messages](LogMessageConsumer* c) {
		  messages.insert(c->message());
		}
  );
  ASSERT_EQ(messages.size(), 100);

  numFromPool= 0;
  for (auto i= messages.begin(); i != messages.end(); ++i) {
    if (((*i)->capacity() != INITIAL_CAPACITY) ||
	((*i)->maxCapacity() != MAX_CAPACITY) ||
	!((*i)->empty())) {
      badCapacity.push_back(*i);
    }
    if (messagesInPool.find(*i) != messagesInPool.end()) {
      ++numFromPool;
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
  EXPECT_EQ(numFromPool, messagesInPool.size());

  // Have 100 threads return messages at once.  At least MAX_POOL_SIZE of
  // them should wind up in the pool with no duplicates.
  releaseGate.open();
  if (!releaseGate.waitUntilClear(DEADLINE)) {
    EXPECT_TRUE(false) << "All threads did not clear the release gate "
		       << "before the deadline";
    return;
  }
  exitGate.waitUntilArrived(100, DEADLINE);
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

  // Verify that the pool has MAX_POOL_SIZE messages that came from "messages"
  // and that there are no duplicates
  numFromPool= 0;
  messagesInPool.clear();
  factory.getMessagesInPool(plainInserter(messagesInPool));
  EXPECT_EQ(messagesInPool.size(), MAX_POOL_SIZE);
  for (auto i=messagesInPool.begin(); i != messagesInPool.end(); ++i) {
    if (messages.find(*i) != messages.end()) {
      ++numFromPool;
    }
  }
  EXPECT_EQ(numFromPool, MAX_POOL_SIZE);
}

/** @brief Test simultaneous acquistion and release of messages from
 *         multiple threads.
 *
 *  This test spawns two sets of 100 consumer threads each.  The first set
 *  acquires messages simultaneously from the pool.  Then this set returns
 *  all 100 messages to the pool simultaneously, while at the same time,
 *  the second set tries to acquire 100 messages.  Finally, the second set
 *  returns its messages to the pool.  Each set of messages acquired is
 *  checked to ensure there are no duplicates and that each message has
 *  the correct capacities and is empty.  We don't test how many messages
 *  came from the pool and how many came from the heap because (a) outside
 *  of the first set of 100, this number is nondeterministic, and (b) this
 *  has already been tested in SimultaneousGetTest.
 */
TEST(LogMessagePoolTests, SimultaneousGetAndRelease) {
  static const size_t INITIAL_CAPACITY=128;
  static const size_t MAX_CAPACITY= 1024;
  static const size_t MAX_RETURNED_MESSAGE_SIZE= 256;
  static const size_t INITIAL_POOL_SIZE= 4;
  static const size_t MAX_POOL_SIZE= 90;
  static const std::chrono::system_clock::time_point DEADLINE=
    std::chrono::system_clock::now() + std::chrono::seconds(10);
  TestingLogMessagePool factory(INITIAL_CAPACITY, MAX_CAPACITY,
				MAX_RETURNED_MESSAGE_SIZE, INITIAL_POOL_SIZE,
				MAX_POOL_SIZE);
  LockingGate acquireGate;   ///< Acquire gate for consumers1
  LockingGate releaseGate1;  ///< Also the acquire gate for consumers2
  LockingGate releaseGate2;  ///< Also the exit gate for consumers1
  LockingGate exitGate;      ///< Exit gate for consumers2
  ThreadSet<LogMessageConsumer> consumers1(100, 1, &factory, &acquireGate,
					   &releaseGate1, &releaseGate2,
					   DEADLINE);
  ThreadSet<LogMessageConsumer> consumers2(100, 101, &factory, &releaseGate1,
					   &releaseGate2, &exitGate, DEADLINE);
  std::vector<std::string> errors;
  std::set<LogMessage*> messages;
  std::vector<LogMessage*> badCapacity;
  size_t numLeftInPool;

  // First set of 100 consumers acquires its messages
  if (!acquireGate.waitForThreads(100, DEADLINE)) {
    EXPECT_TRUE(false) << "First set of consumers did not reach their "
		       << "acquire gate before the deadline";
    return;
  }
  acquireGate.open();
  if (!releaseGate1.waitForThreads(200, DEADLINE)) {
    EXPECT_TRUE(false) << "Both sets of consumers did not reach releaseGate1 "
		       << "before the deadline";
    return;
  }

  // At this point, the members of consumers1 have acquired their messages
  // the members of consumers2 are waiting to acquire theirs.  Check consumers1
  // for errors and then proceed.
  errors= consumers1.scanForErrors();
  if (!errors.empty()) {
    std::ostringstream details;
    details << "While acquiring the first set of messages, the following "
	    << "errors occurred in consumers1:\n  "
	    << join(errors.begin(), errors.end(), "  \n");
    EXPECT_TRUE(false) << details.str();
    return;
  }
  
  errors= consumers2.scanForErrors();
  if (!errors.empty()) {
    std::ostringstream details;
    details << "While acquiring the first set of messages, the following "
	    << "errors occurred in consumers2:\n  "
	    << join(errors.begin(), errors.end(), "  \n");
    EXPECT_TRUE(false) << details.str();
    return;
  }

  EXPECT_EQ(factory.numMessagesActive(), 100);
  // Check each member of consumers1 got a unique message and that those
  // messages have the correct capacity.
  std::for_each(consumers1.begin(), consumers1.end(),
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
    } else {
      (*i)->setEnd((*i)->begin() + (*i)->capacity()/2);
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
		  }
    );
    EXPECT_TRUE(false) << details.str();
  }
  
  // First set of consumers releases its messages while the second set
  // tries to acquire them
  releaseGate1.open();
  if (!releaseGate2.waitForThreads(200, DEADLINE)) {
    EXPECT_TRUE(false) << "Both sets of consumers did not reach "
		       << "releaseGate2 before the deadline";
    return;
  }

  // Check for errors in the consumers
  errors= consumers1.scanForErrors();
  if (!errors.empty()) {
    std::ostringstream details;
    details << "While releasing the first set of messages, the following errors occurred in consumers1:\n  "
	    << join(errors.begin(), errors.end(), "  \n");
    EXPECT_TRUE(false) << details.str();
    return;
  }
  
  errors= consumers2.scanForErrors();
  if (!errors.empty()) {
    std::ostringstream details;
    details << "While acquiring the second set of messages, the following "
	    << "errors occurred in consumers2:\n  "
	    << join(errors.begin(), errors.end(), "  \n");
    EXPECT_TRUE(false) << details.str();
    return;
  }

  EXPECT_EQ(factory.numMessagesActive(), 100);
  // Check that each member of the second set of consumers has a unique
  // empty message with the correct capacities.
  messages.clear();
  badCapacity.clear();
  std::for_each(consumers2.begin(), consumers2.end(),
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
  numLeftInPool= factory.numMessagesInPool();
  std::cout << "After simultaneous get and release, there are "
	    << numLeftInPool << " messages in the pool" << std::endl;

  // Now release the second set of messages
  releaseGate2.open();
  exitGate.waitUntilArrived(100, DEADLINE);
  exitGate.open();
  
  // Check that all messages have been released.  There is no way to know
  // exactly how many pushes and pops were executed inside the pool, since
  // this depends upon the exact sequence of operations during the
  // simultaneous get and release of messages.  However, since the pool
  // starts with INITIAL_POOL_SIZE messages, is empty after the first set
  // set of messages are released, it should have at least
  // INITIAL_POOL_SIZE+MAX_POOL_SIZE internal operations done on it.
  EXPECT_EQ(factory.numMessagesActive(), 0);
  EXPECT_EQ(factory.numMessagesInPool(), MAX_POOL_SIZE);

}

TEST(LogMessagePoolTests, Wait) {
  static const size_t INITIAL_CAPACITY=128;
  static const size_t MAX_CAPACITY= 1024;
  static const size_t MAX_RETURNED_MESSAGE_SIZE= 256;
  static const size_t INITIAL_POOL_SIZE= 4;
  static const size_t MAX_POOL_SIZE= 90;
  static const std::chrono::system_clock::time_point DEADLINE=
    std::chrono::system_clock::now() + std::chrono::seconds(10);
  TestingLogMessagePool factory(INITIAL_CAPACITY, MAX_CAPACITY,
				MAX_RETURNED_MESSAGE_SIZE, INITIAL_POOL_SIZE,
				MAX_POOL_SIZE);
  LockingGate acquireGate;
  LockingGate releaseGate;
  LockingGate exitGate;
  LockingGate waitGate1;
  LockingGate waitGate2;
  LockingGate exitGate1;
  LockingGate exitGate2;
  ThreadSet<LogMessageConsumer> consumers(100, 1, &factory, &acquireGate,
					  &releaseGate, &exitGate, DEADLINE);
  ThreadSet<LogMessageWaiter> waiters(10, 1, &factory, &waitGate1,
				      &exitGate1, DEADLINE);
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
    EXPECT_TRUE(false) << "All threads did not reach the release gate "
		       << "before the deadline";
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
    details << "While waiting on a short deadline, the following errors "
	    << "occurred:\n  " << join(errors.begin(), errors.end(), "  \n");
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
    details << notFound.size() << " waiters incorrectly notified the caller "
	    << "that all messages returned to the factory";
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
