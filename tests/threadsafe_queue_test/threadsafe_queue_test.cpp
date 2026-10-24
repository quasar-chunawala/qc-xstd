#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include <core/concurrency/threadsafe_queue.h>

// Test default constructor
TEST(ThreadSafeQueueTest, DefaultConstructorTest) {
	xstd::threadsafe_queue<int> queue;
	EXPECT_EQ(queue.empty(), true);
	EXPECT_EQ(queue.size(), 0);
}

// Test push and front
TEST(ThreadSafeQueueTest, PushAndFrontTest) {
	xstd::threadsafe_queue<int> queue;
	queue.push(42);
	EXPECT_EQ(queue.front(), 42);
	EXPECT_EQ(queue.size(), 1);
	EXPECT_EQ(queue.empty(), false);
}

// Test push and back
TEST(ThreadSafeQueueTest, PushAndBackTest) {
	xstd::threadsafe_queue<int> queue;
	queue.push(10);
	queue.push(20);
	EXPECT_EQ(queue.back(), 20);
	EXPECT_EQ(queue.size(), 2);
}

// Test try_pop (non-blocking)
TEST(ThreadSafeQueueTest, TryPopTest) {
	xstd::threadsafe_queue<int> queue;
	queue.push(42);
	auto item = queue.try_pop();
	EXPECT_TRUE(item.has_value());
	EXPECT_EQ(item.value(), 42);
	EXPECT_EQ(queue.empty(), true);
}

// Test pop (blocking)
TEST(ThreadSafeQueueTest, BlockingPopTest) {
	xstd::threadsafe_queue<int> queue;

	std::thread producer([&queue]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		queue.push(42);
	});

	auto item = queue.pop();
	EXPECT_EQ(item, 42);
	EXPECT_EQ(queue.empty(), true);

	producer.join();
}

// Test emplace
TEST(ThreadSafeQueueTest, EmplaceTest) {
	struct Point {
		int x, y;
		Point(int a, int b) : x(a), y(b) {}
	};

	xstd::threadsafe_queue<Point> queue;
	queue.emplace(1, 2);
	EXPECT_EQ(queue.front().x, 1);
	EXPECT_EQ(queue.front().y, 2);
}

// Test thread safety with multiple producers and consumers
TEST(ThreadSafeQueueTest, MultiThreadedTest) {
	xstd::threadsafe_queue<int> queue;
	const int num_items = 100;

	std::thread producer1([&queue]() {
		for (int i = 0; i < num_items; ++i) {
			queue.push(i);
		}
	});

	std::thread producer2([&queue]() {
		for (int i = num_items; i < 2 * num_items; ++i) {
			queue.push(i);
		}
	});

	std::vector<int> consumed_items;
	std::mutex mtx;
	std::thread consumer1([&queue, &consumed_items, &mtx]() {
		for (int i = 0; i < num_items; ++i) {
			{
				std::unique_lock<std::mutex> unique_lck(mtx);
				consumed_items.push_back(queue.pop());
			}
		}
	});

	std::thread consumer2([&queue, &consumed_items, &mtx]() {
		for (int i = 0; i < num_items; ++i) {
			{
				std::unique_lock<std::mutex> unique_lck(mtx);
				consumed_items.push_back(queue.pop());
			}
		}
	});

	producer1.join();
	producer2.join();
	consumer1.join();
	consumer2.join();

	EXPECT_EQ(consumed_items.size(), 2 * num_items);
	EXPECT_EQ(queue.empty(), true);
}

// Test empty queue behavior
TEST(ThreadSafeQueueTest, EmptyQueueTest) {
	xstd::threadsafe_queue<int> queue;
	EXPECT_EQ(queue.empty(), true);
	EXPECT_EQ(queue.size(), 0);

	auto item = queue.try_pop();
	EXPECT_FALSE(item.has_value());
}

// Test copy constructor
TEST(ThreadSafeQueueTest, CopyConstructorTest) {
	xstd::threadsafe_queue<int> queue1;
	queue1.push(42);
	queue1.push(17);

	xstd::threadsafe_queue<int> queue2(queue1);
	EXPECT_EQ(queue2.size(), 2);
	EXPECT_EQ(queue2.front(), 42);
	EXPECT_EQ(queue2.back(), 17);
}

// Test size
TEST(ThreadSafeQueueTest, SizeTest) {
	xstd::threadsafe_queue<int> queue;
	queue.push(1);
	queue.push(2);
	queue.push(3);
	EXPECT_EQ(queue.size(), 3);
}