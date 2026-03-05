#ifndef INCLUDE_UTILITY_HPP
#define INCLUDE_UTILITY_HPP

#include <fstream>
#include <string>

namespace Framework::Utility {
	inline bool create_file(const std::string &name) {
		std::ofstream file(name);
		bool const opened = file.is_open();
		file.close();
		return opened;
	}
} // namespace Framework::Utility

#endif // INCLUDE_UTILITY_HPP
