#ifndef TESTS_MESSAGETEST_SYNCHRONIZEDDEQUETEST_HPP
#define TESTS_MESSAGETEST_SYNCHRONIZEDDEQUETEST_HPP

#include <gtest/gtest.h>
#include "Message/SynchronizedDeque.hpp"

using namespace Framework::Message;

class SynchronizedDequeTest : public ::testing::Test {
protected:
	SynchronizedDeque<int> queue_;
};

#endif // TESTS_MESSAGETEST_SYNCHRONIZEDDEQUETEST_HPP
