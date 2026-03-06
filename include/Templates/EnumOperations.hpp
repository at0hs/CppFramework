#ifndef INCLUDE_TEMPLATES_ENUMOPERATIONS_HPP
#define INCLUDE_TEMPLATES_ENUMOPERATIONS_HPP

#include "EnumCast.hpp"
#include <type_traits>

namespace Framework::Templates::EnumOperations {

	template <typename T>
	inline constexpr bool kHasBitwiseOperations = false;

	namespace Concept {
		template <typename T>
		concept EnumBitwiseSafe = kHasBitwiseOperations<T> && std::is_enum_v<T>;
	} // namespace Concept

	template <Concept::EnumBitwiseSafe T>
	static constexpr T operator&(T lhs, T rhs) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(Cast::to_underlying(lhs) & Cast::to_underlying(rhs));
	}

	template <Concept::EnumBitwiseSafe T>
	static constexpr T &operator&=(T &lhs, T rhs) {
		using Cast = EnumCast<T>;
		lhs = Cast::to_enum(Cast::to_underlying(lhs) & Cast::to_underlying(rhs));
		return lhs;
	}

	template <Concept::EnumBitwiseSafe T>
	static constexpr T operator|(T lhs, T rhs) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(Cast::to_underlying(lhs) | Cast::to_underlying(rhs));
	}

	template <Concept::EnumBitwiseSafe T>
	static constexpr T &operator|=(T &lhs, T rhs) {
		using Cast = EnumCast<T>;
		lhs = Cast::to_enum(Cast::to_underlying(lhs) | Cast::to_underlying(rhs));
		return lhs;
	}

	template <Concept::EnumBitwiseSafe T>
	static constexpr T operator~(T value) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(~Cast::to_underlying(value));
	}

	template <Concept::EnumBitwiseSafe T>
	static constexpr T operator^(T lhs, T rhs) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(Cast::to_underlying(lhs) ^ Cast::to_underlying(rhs));
	}

	template <Concept::EnumBitwiseSafe T>
	static constexpr T &operator^=(T &lhs, T rhs) {
		using Cast = EnumCast<T>;
		lhs = Cast::to_enum(Cast::to_underlying(lhs) ^ Cast::to_underlying(rhs));
		return lhs;
	}
} // namespace Framework::Templates::EnumOperations
#endif // INCLUDE_TEMPLATES_ENUMOPERATIONS_HPP
