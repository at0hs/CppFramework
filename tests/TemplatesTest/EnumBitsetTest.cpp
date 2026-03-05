#include "EnumBitsetTest.hpp"

using namespace Framework::Templates;

TEST_F(EnumBitsetTest, DefaultConstructor) {
	EXPECT_TRUE(bitset_.none());
}

TEST_F(EnumBitsetTest, InitializerListConstructor) {
	EnumBitset<BitsetTestEnum> bitset({ BitsetTestEnum::Value1, BitsetTestEnum::Value3 });

	EXPECT_TRUE(bitset.test(BitsetTestEnum::Value1));
	EXPECT_TRUE(bitset.test(BitsetTestEnum::Value3));
	EXPECT_FALSE(bitset.test(BitsetTestEnum::Value2));
}

TEST_F(EnumBitsetTest, SingleValueConstructor) {
	EnumBitset<BitsetTestEnum> bitset(BitsetTestEnum::Value2);
	EXPECT_TRUE(bitset.test(BitsetTestEnum::Value2));
	EXPECT_FALSE(bitset.test(BitsetTestEnum::Value1));
}

TEST_F(EnumBitsetTest, AndOperator) {
	EnumBitset<BitsetTestEnum> bitset1({ BitsetTestEnum::Value1, BitsetTestEnum::Value2 });
	EnumBitset<BitsetTestEnum> bitset2({ BitsetTestEnum::Value2, BitsetTestEnum::Value3 });
	bitset1 &= bitset2;
	EXPECT_FALSE(bitset1.test(BitsetTestEnum::Value1));
	EXPECT_TRUE(bitset1.test(BitsetTestEnum::Value2));
	EXPECT_FALSE(bitset1.test(BitsetTestEnum::Value3));
}

TEST_F(EnumBitsetTest, OrOperator) {
	EnumBitset<BitsetTestEnum> bitset1({ BitsetTestEnum::Value1 });
	EnumBitset<BitsetTestEnum> bitset2({ BitsetTestEnum::Value2 });
	bitset1 |= bitset2;
	EXPECT_TRUE(bitset1.test(BitsetTestEnum::Value1));
	EXPECT_TRUE(bitset1.test(BitsetTestEnum::Value2));
}

TEST_F(EnumBitsetTest, XorOperator) {
	EnumBitset<BitsetTestEnum> bitset1({ BitsetTestEnum::Value1, BitsetTestEnum::Value2 });
	EnumBitset<BitsetTestEnum> bitset2({ BitsetTestEnum::Value2, BitsetTestEnum::Value3 });
	bitset1 ^= bitset2;
	EXPECT_TRUE(bitset1.test(BitsetTestEnum::Value1));
	EXPECT_FALSE(bitset1.test(BitsetTestEnum::Value2));
	EXPECT_TRUE(bitset1.test(BitsetTestEnum::Value3));
}

TEST_F(EnumBitsetTest, SetAll) {
	bitset_.set();
	EXPECT_TRUE(bitset_.all());
}

TEST_F(EnumBitsetTest, SetSingle) {
	bitset_.set(BitsetTestEnum::Value1);
	EXPECT_TRUE(bitset_.test(BitsetTestEnum::Value1));
	EXPECT_FALSE(bitset_.test(BitsetTestEnum::Value2));
}

TEST_F(EnumBitsetTest, ResetAll) {
	bitset_.set();
	bitset_.reset();
	EXPECT_TRUE(bitset_.none());
}

TEST_F(EnumBitsetTest, ResetSingle) {
	bitset_.set(BitsetTestEnum::Value1);
	bitset_.reset(BitsetTestEnum::Value1);
	EXPECT_FALSE(bitset_.test(BitsetTestEnum::Value1));
}

TEST_F(EnumBitsetTest, FlipAll) {
	bitset_.flip();
	EXPECT_TRUE(bitset_.all());
}

TEST_F(EnumBitsetTest, FlipSingle) {
	bitset_.flip(BitsetTestEnum::Value1);
	EXPECT_TRUE(bitset_.test(BitsetTestEnum::Value1));
	bitset_.flip(BitsetTestEnum::Value1);
	EXPECT_FALSE(bitset_.test(BitsetTestEnum::Value1));
}

TEST_F(EnumBitsetTest, Count) {
	bitset_.set(BitsetTestEnum::Value1);
	bitset_.set(BitsetTestEnum::Value2);
	EXPECT_EQ(2, bitset_.count());
}

TEST_F(EnumBitsetTest, Any) {
	bitset_.set(BitsetTestEnum::Value1);
	EXPECT_TRUE(bitset_.any());
}

TEST_F(EnumBitsetTest, None) {
	EXPECT_TRUE(bitset_.none());
}

TEST_F(EnumBitsetTest, EqualityOperator) {
	EnumBitset<BitsetTestEnum> bitset1({ BitsetTestEnum::Value1 });
	EnumBitset<BitsetTestEnum> bitset2({ BitsetTestEnum::Value1 });
	EXPECT_TRUE(bitset1 == bitset2);
}

TEST_F(EnumBitsetTest, InequalityOperator) {
	EnumBitset<BitsetTestEnum> bitset1({ BitsetTestEnum::Value1 });
	EnumBitset<BitsetTestEnum> bitset2({ BitsetTestEnum::Value2 });
	EXPECT_TRUE(bitset1 != bitset2);
}

TEST_F(EnumBitsetTest, ToUnderlying) {
	EnumBitset<BitsetTestEnum> bitset({ BitsetTestEnum::Value1, BitsetTestEnum::Value3 });
	EXPECT_EQ(static_cast<typename EnumBitset<BitsetTestEnum>::UnderlyingType>(BitsetTestEnum::Value1) |
		static_cast<typename EnumBitset<BitsetTestEnum>::UnderlyingType>(BitsetTestEnum::Value3),
		bitset.to_underlying());
}
