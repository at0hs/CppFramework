#include "PropertyTest.hpp"
#include "Templates/Property.hpp"
#include <sstream>
#include <stdexcept>

using namespace Framework::Templates;

namespace {
	class Getter {
	public:
		static int get(const int &value) noexcept { return value * 2; }
	};

	struct PropertyHours {
		struct Setter {
			static void set(double &value, const double &new_value) {
				if (new_value < 0 || new_value > 24) {
					throw std::out_of_range("The valid range is between 0 and 24.");
				}
				value = new_value * 3600;
			}
		};
		struct Getter {
			static double get(const double &value) noexcept { return value / 3600; }
		};
	};

	class FunctionSetterTest {
		std::function<int(int)> function_;

	public:
		ReferenceProperty::FunctionSetter<int(int)> function{ function_ };
		int invoke(int value) { return function_(value); }
	};
} // namespace

TEST_F(PropertyTest, ReferenceProperty_ReadOnly_IntVariable) {
	int value = 42;
	ReferenceProperty::ReadOnly<int> property(value);
	const int &actual = property;
	// property = 0; // Error: cannot assign to a read-only property
	EXPECT_EQ(42, actual);
}

TEST_F(PropertyTest, ReferenceProperty_ReadOnly_ConstVariable) {
	const int value = 42;
	ReferenceProperty::ReadOnly<int> property(value);
	int actual = property;
	EXPECT_EQ(42, actual);
}

TEST_F(PropertyTest, ReferenceProperty_ReadOnly_CustomGetter) {
	int value = 42;
	constexpr int expected = 42 * 2;
	ReferenceProperty::ReadOnly<int, Getter> property(value);
	int actual = property;
	EXPECT_EQ(expected, actual);
}

TEST_F(PropertyTest, ReferenceProperty_ReadOnly_OutputStream) {
	int value = 42;
	ReferenceProperty::ReadOnly<int> property(value);
	std::ostringstream oss;
	oss << property;
	EXPECT_EQ("42", oss.str());
}

TEST_F(PropertyTest, ReferenceProperty_Writable_IntVariable) {
	int value = 42;
	ReferenceProperty::Writable<int> property(value);

	int actual = property;
	EXPECT_EQ(42, actual);
	property = 0;
	int actual2 = property;
	EXPECT_EQ(0, actual2);
}

TEST_F(PropertyTest, ReferenceProperty_Writable_CustomGetterAndSetter) {
	double seconds = 0;
	ReferenceProperty::Writable<double, PropertyHours::Getter, PropertyHours::Setter> property(
		seconds);

	property = 12;

	double actual = property;

	EXPECT_EQ(12, actual);
	EXPECT_EQ(12 * 3600, seconds);
}

TEST_F(PropertyTest, FunctionSetter) {
	FunctionSetterTest test;
	test.function = [](int value) {
		return value * 2;
	};
	int actual = test.invoke(42);
	EXPECT_EQ(42 * 2, actual);
}

TEST_F(PropertyTest, Property_Writable) {
	int value = 0;
	Property<int>::Writable writable_property{ [&]() -> int { return value; },
											  [&](const int &new_value) { value = new_value; } };

	writable_property = 10;
	int actual = writable_property;
	EXPECT_EQ(10, actual);
}

TEST_F(PropertyTest, Property_ReadOnly) {
	int value = 10;
	Property<int>::ReadOnly read_only_property{ [&]() -> int { return value; } };

	// readOnlyProperty = 20; // Error: cannot assign to a read-only property
	int actual = read_only_property;
	EXPECT_EQ(10, actual);
}
