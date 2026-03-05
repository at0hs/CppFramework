#include "SubProcess/StartInfo.hpp"

namespace Framework::SubProcess {

	StartInfo::StartInfo(std::string command) : command_(std::move(command)) {}

	std::string &StartInfo::command() {
		return command_;
	}

	const std::string &StartInfo::command() const {
		return command_;
	}

	std::vector<std::string> &StartInfo::arguments() {
		return arguments_;
	}

	const std::vector<std::string> &StartInfo::arguments() const {
		return arguments_;
	}

	std::vector<std::string> &StartInfo::environments() {
		return environments_;
	}

	const std::vector<std::string> &StartInfo::environments() const {
		return environments_;
	}

	bool &StartInfo::redirect_standard_output() {
		return redirect_standard_output_;
	}

	bool StartInfo::redirect_standard_output() const {
		return redirect_standard_output_;
	}

	bool &StartInfo::redirect_standard_error() {
		return redirect_standard_error_;
	}

	bool StartInfo::redirect_standard_error() const {
		return redirect_standard_error_;
	}

	bool &StartInfo::use_shell() {
		return use_shell_;
	}

	bool StartInfo::use_shell() const {
		return use_shell_;
	}

	std::string StartInfo::get_command_line() const {
		std::string result = command_;
		for (const auto &argument : arguments_) {
			result += " " + argument;
		}
		return result;
	}

	std::vector<const char *> StartInfo::c_style_environments() const {
		std::vector<const char *> result;
		result.reserve(environments_.size() + 1);
		for (const auto &env : environments_) {
			result.push_back(env.c_str());
		}
		result.push_back(nullptr);
		return result;
	}
} // namespace Framework::SubProcess
