#ifndef INCLUDE_TEMPLATES_ENUMOPERATIONS_HPP
#define INCLUDE_TEMPLATES_ENUMOPERATIONS_HPP

#include "EnumCast.hpp"
#include <type_traits>

namespace Framework::Templates::EnumOperations {

	namespace Concept {
		template <typename T>
		struct HasBitwiseOperators : std::false_type {};

		template <typename T, typename U>
		concept EnumBitwiseSafe =
			HasBitwiseOperators<T>::value && std::is_same_v<T, U> && std::is_enum_v<T>;
	} // namespace Concept

	template <typename T, typename U = T>
		requires Concept::EnumBitwiseSafe<T, U>
	static constexpr T operator&(T lhs, U rhs) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(Cast::to_underlying(lhs) & Cast::to_underlying(rhs));
	}

	template <typename T, typename U = T>
		requires Concept::EnumBitwiseSafe<T, U>
	static constexpr T &operator&=(T &lhs, U rhs) {
		using Cast = EnumCast<T>;
		lhs = Cast::to_enum(Cast::to_underlying(lhs) & Cast::to_underlying(rhs));
		return lhs;
	}

	template <typename T, typename U = T>
		requires Concept::EnumBitwiseSafe<T, U>
	static constexpr T operator|(T lhs, U rhs) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(Cast::to_underlying(lhs) | Cast::to_underlying(rhs));
	}

	template <typename T, typename U = T>
		requires Concept::EnumBitwiseSafe<T, U>
	static constexpr T &operator|=(T &lhs, U rhs) {
		using Cast = EnumCast<T>;
		lhs = Cast::to_enum(Cast::to_underlying(lhs) | Cast::to_underlying(rhs));
		return lhs;
	}

	template <typename T, typename U = T>
		requires Concept::EnumBitwiseSafe<T, U>
	static constexpr T operator~(T value) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(~Cast::to_underlying(value));
	}

	template <typename T, typename U = T>
		requires Concept::EnumBitwiseSafe<T, U>
	static constexpr T operator^(T lhs, U rhs) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(Cast::to_underlying(lhs) ^ Cast::to_underlying(rhs));
	}

	template <typename T, typename U = T>
		requires Concept::EnumBitwiseSafe<T, U>
	static constexpr T &operator^=(T &lhs, U rhs) {
		using Cast = EnumCast<T>;
		lhs = Cast::to_enum(Cast::to_underlying(lhs) ^ Cast::to_underlying(rhs));
		return lhs;
	}
} // namespace Framework::Templates::EnumOperations
#endif // INCLUDE_TEMPLATES_ENUMOPERATIONS_HPP
