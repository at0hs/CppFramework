#include "EnumOperationsTest.hpp"

TEST_F(EnumOperationsTest, BitwiseAnd) {
	EnumOperationsTestEnum result = EnumOperationsTestEnum::Flag1 & EnumOperationsTestEnum::Flag2;
	EXPECT_EQ(0, Cast::to_underlying(result));

	// NOLINTNEXTLINE(misc-redundant-expression)
	result = EnumOperationsTestEnum::Flag1 & EnumOperationsTestEnum::Flag1;
	EXPECT_EQ(Cast::to_underlying(EnumOperationsTestEnum::Flag1), Cast::to_underlying(result));
}

TEST_F(EnumOperationsTest, BitwiseAndAssignment) {
	EnumOperationsTestEnum value = EnumOperationsTestEnum::Flag1;
	value &= EnumOperationsTestEnum::Flag2;
	EXPECT_EQ(0, Cast::to_underlying(value));

	value = EnumOperationsTestEnum::Flag1;
	value &= EnumOperationsTestEnum::Flag1;
	EXPECT_EQ(Cast::to_underlying(EnumOperationsTestEnum::Flag1), Cast::to_underlying(value));
}

TEST_F(EnumOperationsTest, BitwiseOr) {
	EnumOperationsTestEnum result = EnumOperationsTestEnum::Flag1 | EnumOperationsTestEnum::Flag2;
	EXPECT_EQ(Cast::to_underlying(EnumOperationsTestEnum::Flag1) |
				  Cast::to_underlying(EnumOperationsTestEnum::Flag2),
			  Cast::to_underlying(result));

	// NOLINTNEXTLINE(misc-redundant-expression)
	result = EnumOperationsTestEnum::Flag1 | EnumOperationsTestEnum::Flag1;
	EXPECT_EQ(Cast::to_underlying(EnumOperationsTestEnum::Flag1), Cast::to_underlying(result));
}

TEST_F(EnumOperationsTest, BitwiseOrAssignment) {
	EnumOperationsTestEnum value = EnumOperationsTestEnum::Flag1;
	value |= EnumOperationsTestEnum::Flag2;
	EXPECT_EQ(Cast::to_underlying(EnumOperationsTestEnum::Flag1) |
				  Cast::to_underlying(EnumOperationsTestEnum::Flag2),
			  Cast::to_underlying(value));

	value = EnumOperationsTestEnum::Flag1;
	value |= EnumOperationsTestEnum::Flag1;
	EXPECT_EQ(Cast::to_underlying(EnumOperationsTestEnum::Flag1), Cast::to_underlying(value));
}

TEST_F(EnumOperationsTest, BitwiseNot) {
	EnumOperationsTestEnum result = ~EnumOperationsTestEnum::Flag1;
	// ~uint8_t は int に昇格するため、期待値も uint8_t に明示的にキャストして比較する
	EXPECT_EQ(static_cast<uint8_t>(~Cast::to_underlying(EnumOperationsTestEnum::Flag1)),
			  Cast::to_underlying(result));
}

TEST_F(EnumOperationsTest, BitwiseXor) {
	EnumOperationsTestEnum result = EnumOperationsTestEnum::Flag1 ^ EnumOperationsTestEnum::Flag2;
	EXPECT_EQ(Cast::to_underlying(EnumOperationsTestEnum::Flag1) ^
				  Cast::to_underlying(EnumOperationsTestEnum::Flag2),
			  Cast::to_underlying(result));

	// NOLINTNEXTLINE(misc-redundant-expression)
	result = EnumOperationsTestEnum::Flag1 ^ EnumOperationsTestEnum::Flag1;
	EXPECT_EQ(0, Cast::to_underlying(result));
}

TEST_F(EnumOperationsTest, BitwiseXorAssignment) {
	EnumOperationsTestEnum value = EnumOperationsTestEnum::Flag1;
	value ^= EnumOperationsTestEnum::Flag2;
	EXPECT_EQ(Cast::to_underlying(EnumOperationsTestEnum::Flag1) ^
				  Cast::to_underlying(EnumOperationsTestEnum::Flag2),
			  Cast::to_underlying(value));

	value = EnumOperationsTestEnum::Flag1;
	value ^= EnumOperationsTestEnum::Flag1;
	EXPECT_EQ(0, Cast::to_underlying(value));
}
