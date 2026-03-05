#include "Exception/SystemException.hpp"
#include <cerrno>
#include <cstring>
#include <string>
#include <array>

namespace Framework {
	
	SystemException::SystemException(const std::string &message, int errno_code, Error::Code code)
		: Exception(message, code),
		  errno_code_(errno_code) {}

	int SystemException::errno_code() const noexcept {
		return errno_code_;
	}

	std::string SystemException::errno_message() const {
		std::array<char, 256> buf{};

		// buf.data() で生のポインタを取得し、buf.size() でサイズを渡す
		if (strerror_r(errno_code_, buf.data(), buf.size()) == 0) {
			return  std::string(buf.data());
		}
		return "Unknown error";
	}

} // namespace Framework
