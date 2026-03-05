#ifndef TESTS_SYNCTEST_EVENTFLAGTEST_HPP
#define TESTS_SYNCTEST_EVENTFLAGTEST_HPP

#include "gtest/gtest.h"
#include "Sync/EventFlag.hpp"

class EventFlagTest : public ::testing::Test {
protected:
	EFlag::EventFlag event_flag_;
};

#endif // TESTS_SYNCTEST_EVENTFLAGTEST_HPP
