#ifndef INCLUDE_TEMPLATES_ENUMOPERATIONS_HPP
#define INCLUDE_TEMPLATES_ENUMOPERATIONS_HPP

#include "EnumCast.hpp"
#include <cstddef>
#include <type_traits>

namespace Framework::Templates::EnumOperations {

	namespace Concept {
		template <typename T>
		struct HasBitwiseOperators : std::false_type {};
	} // namespace Concept

	template <typename T, typename U>
	struct AreEnumLogicSafety : public std::conditional_t<std::is_same_v<T, U> && std::is_enum_v<T>,
														  std::true_type, std::false_type> {};

	template <typename T, typename U>
	using EnumBitwiseEnabler =
		std::enable_if<Concept::HasBitwiseOperators<T>::value && AreEnumLogicSafety<T, U>::value,
					   std::nullptr_t>;

	template <typename T, typename U = T, typename EnumBitwiseEnabler<T, U>::type = nullptr>
	static constexpr T operator&(T lhs, U rhs) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(Cast::to_underlying(lhs) & Cast::to_underlying(rhs));
	}
	template <typename T, typename U = T, typename EnumBitwiseEnabler<T, U>::type = nullptr>
	static constexpr T &operator&=(T &lhs, U rhs) {
		using Cast = EnumCast<T>;
		lhs = Cast::to_enum(Cast::to_underlying(lhs) & Cast::to_underlying(rhs));
		return lhs;
	}

	template <typename T, typename U = T, typename EnumBitwiseEnabler<T, U>::type = nullptr>
	static constexpr T operator|(T lhs, U rhs) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(Cast::to_underlying(lhs) | Cast::to_underlying(rhs));
	}

	template <typename T, typename U = T, typename EnumBitwiseEnabler<T, U>::type = nullptr>
	static constexpr T &operator|=(T &lhs, U rhs) {
		using Cast = EnumCast<T>;
		lhs = Cast::to_enum(Cast::to_underlying(lhs) | Cast::to_underlying(rhs));
		return lhs;
	}

	template <typename T, typename U = T, typename EnumBitwiseEnabler<T, U>::type = nullptr>
	static constexpr T operator~(T value) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(~Cast::to_underlying(value));
	}

	template <typename T, typename U = T, typename EnumBitwiseEnabler<T, U>::type = nullptr>
	static constexpr T operator^(T lhs, U rhs) {
		using Cast = EnumCast<T>;
		return Cast::to_enum(Cast::to_underlying(lhs) ^ Cast::to_underlying(rhs));
	}

	template <typename T, typename U = T, typename EnumBitwiseEnabler<T, U>::type = nullptr>
	static constexpr T &operator^=(T &lhs, U rhs) {
		using Cast = EnumCast<T>;
		lhs = Cast::to_enum(Cast::to_underlying(lhs) ^ Cast::to_underlying(rhs));
		return lhs;
	}
} // namespace Framework::Templates::EnumOperations
#endif // INCLUDE_TEMPLATES_ENUMOPERATIONS_HPP
