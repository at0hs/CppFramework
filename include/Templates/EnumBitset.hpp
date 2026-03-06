#ifndef INCLUDE_TEMPLATES_ENUMBITSET_HPP
#define INCLUDE_TEMPLATES_ENUMBITSET_HPP

#include "EnumCast.hpp"
#include <bitset>
#include <cstddef>
#include <initializer_list>
#include <string>
#include <type_traits>

namespace Framework::Templates {

	template <typename T, std::size_t MaxBits = sizeof(std::underlying_type_t<T>) * 8>
		requires std::is_enum_v<T> && (MaxBits <= sizeof(std::underlying_type_t<T>) * 8)
	class EnumBitset : std::bitset<MaxBits> {

		using _base = std::bitset<MaxBits>;

	public:
		using EnumCast = class EnumCast<T>;
		using Type = typename EnumCast::Type;
		using UnderlyingType = typename EnumCast::UnderlyingType;

		static constexpr size_t kMaxBits = MaxBits;

		constexpr EnumBitset() noexcept : _base() {}

		constexpr EnumBitset(std::initializer_list<Type> init) noexcept
			: _base(EnumCast::to_underlying(or_all(init))) {}

		constexpr EnumBitset(Type value) noexcept : EnumBitset({ value }) {}

		EnumBitset &operator&=(const EnumBitset &other) noexcept {
			_base::operator&=(other);
			return *this;
		}

		EnumBitset &operator|=(const EnumBitset &other) noexcept {
			_base::operator|=(other);
			return *this;
		}

		EnumBitset &operator^=(const EnumBitset &other) noexcept {
			_base::operator^=(other);
			return *this;
		}

		EnumBitset &set() noexcept {
			_base::set();
			return *this;
		}

		EnumBitset &set(Type e, bool value = true) {
			if (EnumCast::to_underlying(e) == 0) [[unlikely]] {
				return *this;
			}
			_base::set(to_bit_position(e), value);
			return *this;
		}

		EnumBitset &reset() noexcept {
			_base::reset();
			return *this;
		}

		EnumBitset &reset(Type value) {
			if (EnumCast::to_underlying(value) == 0) [[unlikely]] {
				return *this;
			}
			_base::reset(to_bit_position(value));
			return *this;
		}

		EnumBitset operator~() const noexcept { return _base::operator~(); }

		EnumBitset &flip() noexcept {
			_base::flip();
			return *this;
		}

		EnumBitset &flip(Type value) {
			if (EnumCast::to_underlying(value) == 0) [[unlikely]] {
				return *this;
			}
			_base::flip(to_bit_position(value));
			return *this;
		}

		bool operator[](UnderlyingType position) const { return _base::operator[](position); }

		size_t count() const noexcept { return _base::count(); }

		bool test(Type value) const {
			if (EnumCast::to_underlying(value) == 0) [[unlikely]] {
				return false;
			}
			return test(to_bit_position(value));
		}

		bool test(UnderlyingType position) const { return _base::test(position); }

		bool all() const noexcept { return _base::all(); }

		bool any() const noexcept { return _base::any(); }

		bool none() const noexcept { return _base::none(); }

		bool operator==(const EnumBitset &other) const noexcept { return _base::operator==(other); }

		bool operator!=(const EnumBitset &other) const noexcept {
			return !_base::operator==(other);
		}

		UnderlyingType to_underlying() const noexcept { return _base::to_ullong(); }

		Type to_enum() const { return EnumCast::to_enum(to_underlying()); }

		std::string to_string() const { return _base::to_string(); }

		static constexpr Type or_all(std::initializer_list<Type> init) noexcept {
			UnderlyingType result = EnumCast::to_underlying(Type(0));
			for (Type value : init) {
				UnderlyingType underlying = EnumCast::to_underlying(value);
				result |= underlying;
			}
			return EnumCast::to_enum(result);
		}

	private:
		static constexpr UnderlyingType to_bit_position(Type value) {
			return __builtin_ctzll(EnumCast::to_underlying(value));
		}
	};
} // namespace Framework::Templates

#endif // INCLUDE_TEMPLATES_ENUMBITSET_HPP
