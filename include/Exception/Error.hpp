#ifndef INCLUDE_EXCEPTION_ERROR_HPP
#define INCLUDE_EXCEPTION_ERROR_HPP

#include <cstdint>

namespace Framework {
	class Error {
	public:
		enum class Code : int8_t {
			Unknown = -1,
			Success,
			OutOfRange,
			InvalidArgument,
			TypeMismatch,
			InvalidOperation,
			SystemError,
		};
	};
} // namespace Framework

#endif // INCLUDE_EXCEPTION_ERROR_HPP
