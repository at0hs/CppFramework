#ifndef INCLUDE_EXCEPTION_EXCEPTION_HPP
#define INCLUDE_EXCEPTION_EXCEPTION_HPP

#include "Error.hpp"
#include <any>
#include <optional>
#include <stdexcept>
#include <string>

namespace Framework {
	class Exception : public std::runtime_error {
		Error::Code code_;
		std::optional<std::any> context_;

		static std::string build_error_message(const std::string &message, Error::Code code);

	public:
		explicit Exception(const std::string &message, Error::Code code = Error::Code::Unknown);
		Exception(const char *message, Error::Code code = Error::Code::Unknown);
		Exception(const std::string &message, Error::Code code, std::any context);

		Error::Code get_code() const;
		bool has_context() const noexcept;

		template <typename T>
		std::optional<T> get_context() const noexcept {
			if (!context_) {
				return std::nullopt;
			}
			const T *ptr = std::any_cast<T>(&*context_);
			return ptr ? std::optional<T>{ *ptr } : std::nullopt;
		}
	};
} // namespace Framework

#endif // INCLUDE_EXCEPTION_EXCEPTION_HPP
