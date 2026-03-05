#ifndef TESTS_SUBPROCESSTEST_PROCESSTEST_HPP
#define TESTS_SUBPROCESSTEST_PROCESSTEST_HPP

#include <chrono>

#include "gtest/gtest.h"

class ProcessTest : public ::testing::Test {
protected:
	// kill() テストで長時間プロセスを待つ上限（秒）
	static constexpr auto kKillWaitMs = std::chrono::milliseconds(2000);
};

#endif // TESTS_SUBPROCESSTEST_PROCESSTEST_HPP
