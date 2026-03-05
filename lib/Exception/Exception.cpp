#include "Exception/Exception.hpp"
#include "Exception/Error.hpp"
#include <any>
#include <stdexcept>
#include <string>

namespace Framework {

	std::string Exception::build_error_message(const std::string &message, Error::Code code) {
		return message + " (Error Code: " + std::to_string(static_cast<int>(code)) + ")";
	}

	Exception::Exception(const std::string &message, Error::Code code)
		: std::runtime_error(build_error_message(message, code)),
		  code_(code) {}

	Exception::Exception(const char *message, Error::Code code)
		: std::runtime_error(build_error_message(std::string(message), code)),
		  code_(code) {}

	Exception::Exception(const std::string &message, Error::Code code, std::any context)
		: std::runtime_error(build_error_message(message, code)),
		  code_(code),
		  context_(std::move(context)) {}

	Error::Code Exception::get_code() const {
		return code_;
	}

	bool Exception::has_context() const noexcept {
		return context_.has_value();
	}
} // namespace Framework
