#ifndef INCLUDE_EXCEPTION_SYSTEM_EXCEPTION_HPP
#define INCLUDE_EXCEPTION_SYSTEM_EXCEPTION_HPP

#include "Exception.hpp"
#include <string>

namespace Framework {
	class SystemException : public Exception {
		int errno_code_;

	public:
		SystemException(const std::string &message, int errno_code,
						Error::Code code = Error::Code::SystemError);

		int errno_code() const noexcept;
		std::string errno_message() const;
	};
} // namespace Framework

#endif // INCLUDE_EXCEPTION_SYSTEM_EXCEPTION_HPP
