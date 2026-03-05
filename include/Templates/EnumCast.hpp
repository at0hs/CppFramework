#ifndef INCLUDE_TEMPLATES_ENUMCAST_HPP
#define INCLUDE_TEMPLATES_ENUMCAST_HPP

#include <cstddef>
#include <type_traits>

namespace Framework::Templates {

	template <typename T, typename std::enable_if_t<std::is_enum_v<T>, std::nullptr_t> = nullptr>
	class EnumCast {
	public:
		using Type = T;
		using UnderlyingType = std::underlying_type_t<T>;

		static constexpr Type to_enum(UnderlyingType value) { return static_cast<Type>(value); }

		static constexpr UnderlyingType to_underlying(Type value) {
			return static_cast<UnderlyingType>(value);
		}
	};
} // namespace Framework::Templates
#endif // INCLUDE_TEMPLATES_ENUMCAST_HPP
