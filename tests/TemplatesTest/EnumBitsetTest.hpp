#ifndef TESTS_TEMPLATESTEST_ENUMBITSETTEST_HPP
#define TESTS_TEMPLATESTEST_ENUMBITSETTEST_HPP

#include "gtest/gtest.h"
#include "Templates/EnumBitset.hpp"

enum class BitsetTestEnum: uint8_t {
	Value1 = 1 << 0,
	Value2 = 1 << 1,
	Value3 = 1 << 2,
	Value4 = 1 << 3,
	Value5 = 1 << 4
};

class EnumBitsetTest : public ::testing::Test {
protected:
	Framework::Templates::EnumBitset<BitsetTestEnum> bitset_;
};

#endif // TESTS_TEMPLATESTEST_ENUMBITSETTEST_HPP
