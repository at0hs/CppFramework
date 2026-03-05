#include "SynchronizedDequeTest.hpp"

#include <chrono>
#include <thread>

TEST_F(SynchronizedDequeTest, SendAndReceive) {
	queue_.send(42);
	EXPECT_EQ(42, queue_.receive());
}

TEST_F(SynchronizedDequeTest, SendAndReceiveMultiple) {
	queue_.send(1);
	queue_.send(2);
	queue_.send(3);
	EXPECT_EQ(1, queue_.receive());
	EXPECT_EQ(2, queue_.receive());
	EXPECT_EQ(3, queue_.receive());
}

TEST_F(SynchronizedDequeTest, TimedReceiveSuccess) {
	queue_.send(42);
	auto result = queue_.timed_receive(std::chrono::milliseconds(100));
	EXPECT_TRUE(result.first);
	EXPECT_EQ(42, result.second);
}

TEST_F(SynchronizedDequeTest, TimedReceiveTimeout) {
	auto result = queue_.timed_receive(std::chrono::milliseconds(100));
	EXPECT_FALSE(result.first);
}

TEST_F(SynchronizedDequeTest, ClearQueue) {
	queue_.send(1);
	queue_.send(2);
	queue_.clear();
	EXPECT_TRUE(queue_.is_empty());
}

TEST_F(SynchronizedDequeTest, IsEmpty) {
	EXPECT_TRUE(queue_.is_empty());
	queue_.send(42);
	EXPECT_FALSE(queue_.is_empty());
}

TEST_F(SynchronizedDequeTest, NumMessages) {
	EXPECT_EQ(0, queue_.num_message());
	queue_.send(1);
	queue_.send(2);
	EXPECT_EQ(2, queue_.num_message());
	queue_.receive();
	EXPECT_EQ(1, queue_.num_message());
}

TEST_F(SynchronizedDequeTest, ConcurrentSendAndReceive) {
	std::thread sender([&]() {
		for (int i = 0; i < 10; ++i) {
			queue_.send(i);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	});

	std::thread receiver([&]() {
		for (int i = 0; i < 10; ++i) {
			int value = queue_.receive();
			EXPECT_EQ(i, value);
		}
	});

	sender.join();
	receiver.join();
}
