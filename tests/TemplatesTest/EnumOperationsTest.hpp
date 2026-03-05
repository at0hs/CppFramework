#ifndef TESTS_TEMPLATESTEST_ENUMOPERATIONSTEST_HPP
#define TESTS_TEMPLATESTEST_ENUMOPERATIONSTEST_HPP

#include "Templates/EnumCast.hpp"
#include "Templates/EnumOperations.hpp"
#include "gtest/gtest.h"

using namespace Framework::Templates::EnumOperations;

enum class EnumOperationsTestEnum : uint8_t {
	None = 0,
	Flag1 = 1 << 0,
	Flag2 = 1 << 1,
	Flag3 = 1 << 2,
	All = Flag1 | Flag2 | Flag3
};

namespace Framework::Templates::EnumOperations::Concept {
	template <>
	struct HasBitwiseOperators<EnumOperationsTestEnum> : std::true_type {};
} // namespace Framework::Templates::EnumOperations::Concept

class EnumOperationsTest : public ::testing::Test {
public:
	using Cast = Framework::Templates::EnumCast<EnumOperationsTestEnum>;
};

#endif // TESTS_TEMPLATESTEST_ENUMOPERATIONSTEST_HPP
