#include <gtest/gtest.h>

#include <core/lock_free/spsc_queue.h>

TEST(SPSCQueueTest, PushAndPop) {
	xstd::spsc_queue<int, 6> queue; // Create a queue with capacity 8

	// Push elements onto the queue
	for (int i{0}; i < 7; ++i) {
		queue.try_push(i + 1);
	}

	for (int i{0}; i < 7; ++i) {
		EXPECT_EQ(queue.try_pop(), i + 1);
	}
}