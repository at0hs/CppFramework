#ifndef TESTS_MESSAGETEST_POSIXMESSAGEQUEUETEST_HPP
#define TESTS_MESSAGETEST_POSIXMESSAGEQUEUETEST_HPP

#include <gtest/gtest.h>
#include <mqueue.h>

#include "Message/PosixMessageQueue.hpp"

using namespace Framework::Message;

// テスト間の独立性確保のため、SetUp で残骸をクリーンアップしてから Owner で作成する
class PosixMQTest : public ::testing::Test {
protected:
	static constexpr std::string_view kName = "/fw_test_posix_mq";

	std::unique_ptr<PosixMessageQueue<int>> queue_;

	void SetUp() override {
		mq_unlink(std::string(kName).c_str()); // 前回テストの残骸を除去
		queue_ = std::make_unique<PosixMessageQueue<int>>(kName, Owner{});
	}

	void TearDown() override {
		queue_.reset(); // デストラクタで mq_close + mq_unlink
	}
};

// POD struct を使うテスト用フィクスチャ
struct Point {
	int   x;
	int   y;
	float z;
};

class PosixMQStructTest : public ::testing::Test {
protected:
	static constexpr std::string_view kName = "/fw_test_posix_mq_struct";

	std::unique_ptr<PosixMessageQueue<Point>> queue_;

	void SetUp() override {
		mq_unlink(std::string(kName).c_str());
		queue_ = std::make_unique<PosixMessageQueue<Point>>(kName, Owner{});
	}

	void TearDown() override {
		queue_.reset();
	}
};

#endif // TESTS_MESSAGETEST_POSIXMESSAGEQUEUETEST_HPP
