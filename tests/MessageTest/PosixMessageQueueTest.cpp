#include "PosixMessageQueueTest.hpp"

#include <atomic>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// ============================================================
// 正常系
// ============================================================

TEST_F(PosixMQTest, SendAndReceive) {
	// Arrange + Act
	queue_->send(42);
	const auto value = queue_->receive();

	// Assert
	EXPECT_EQ(42, value);
}

TEST_F(PosixMQTest, FifoOrder) {
	// Arrange
	queue_->send(1);
	queue_->send(2);
	queue_->send(3);

	// Act + Assert: FIFO 順序を確認
	EXPECT_EQ(1, queue_->receive());
	EXPECT_EQ(2, queue_->receive());
	EXPECT_EQ(3, queue_->receive());
}

TEST_F(PosixMQTest, TimedReceiveSuccess) {
	// Arrange
	queue_->send(99);

	// Act
	const auto [ok, value] = queue_->timed_receive(100ms);

	// Assert
	EXPECT_TRUE(ok);
	EXPECT_EQ(99, value);
}

TEST_F(PosixMQTest, NumMessageAfterSend) {
	// Arrange
	EXPECT_EQ(0U, queue_->num_message());

	// Act
	queue_->send(1);
	queue_->send(2);

	// Assert
	EXPECT_EQ(2U, queue_->num_message());

	queue_->receive();
	EXPECT_EQ(1U, queue_->num_message());
}

TEST_F(PosixMQTest, IsEmptyInitially) {
	EXPECT_TRUE(queue_->is_empty());

	queue_->send(0);
	EXPECT_FALSE(queue_->is_empty());
}

TEST_F(PosixMQTest, ClearQueue) {
	// Arrange
	queue_->send(1);
	queue_->send(2);
	queue_->send(3);

	// Act
	queue_->clear();

	// Assert
	EXPECT_TRUE(queue_->is_empty());
}

// ============================================================
// 境界値
// ============================================================

TEST_F(PosixMQStructTest, StructTypeMessage) {
	// Arrange
	const Point sent{ .x = 10, .y = -5, .z = 3.14F };

	// Act
	queue_->send(sent);
	const auto received = queue_->receive();

	// Assert: 各フィールドが一致
	EXPECT_EQ(sent.x, received.x);
	EXPECT_EQ(sent.y, received.y);
	EXPECT_FLOAT_EQ(sent.z, received.z);
}

TEST_F(PosixMQTest, TimedReceiveTimeout) {
	// Act: 空キューで受信を試みる
	const auto [ok, value] = queue_->timed_receive(50ms);

	// Assert
	EXPECT_FALSE(ok);
}

// ============================================================
// 異常系
// ============================================================

TEST(PosixMQErrorTest, InvalidQueueNameThrows) {
	// '/' で始まらない名前は InvalidArgument
	EXPECT_THROW(PosixMessageQueue<int>("no_slash", Owner{}), Framework::Exception);
}

TEST(PosixMQErrorTest, AttachNonExistentQueueThrows) {
	// 存在しないキューへの Attach は SystemError
	mq_unlink("/fw_nonexistent_mq"); // 確実に存在しない状態にする
	EXPECT_THROW(PosixMessageQueue<int>("/fw_nonexistent_mq", Attach{}), Framework::Exception);
}

// ============================================================
// 並行性
// ============================================================

TEST_F(PosixMQTest, MultipleSendersSingleReceiver) {
	constexpr int num_senders = 3;
	constexpr int msg_per_sender = 5;
	constexpr int total = num_senders * msg_per_sender;

	static constexpr std::string_view attach_name = "/fw_test_posix_mq";
	std::atomic<int> received_count{ 0 };

	// 複数スレッドが同じキューに送信（Attach モード）
	std::vector<std::thread> senders;
	senders.reserve(num_senders);
	for (int s = 0; s < num_senders; ++s) {
		senders.emplace_back([&] {
			PosixMessageQueue<int> sender(attach_name, Attach{});
			for (int i = 0; i < msg_per_sender; ++i) {
				sender.send(i);
			}
		});
	}

	// 受信スレッド
	std::thread receiver([&] {
		for (int i = 0; i < total; ++i) {
			const auto [ok, _] = queue_->timed_receive(500ms);
			ASSERT_TRUE(ok) << "受信タイムアウト (i=" << i << ")";
			++received_count;
		}
	});

	for (auto &t : senders) {
		t.join();
	}
	receiver.join();

	EXPECT_EQ(total, received_count.load());
}
