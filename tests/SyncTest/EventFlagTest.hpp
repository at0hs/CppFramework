#ifndef TESTS_SYNCTEST_EVENTFLAGTEST_HPP
#define TESTS_SYNCTEST_EVENTFLAGTEST_HPP

#include "Sync/EventFlag.hpp"
#include "gtest/gtest.h"

class EventFlagTest : public ::testing::Test {
protected:
	EFlag::EventFlag event_flag_;
};

#endif // TESTS_SYNCTEST_EVENTFLAGTEST_HPP
