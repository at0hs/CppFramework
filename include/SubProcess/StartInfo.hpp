#ifndef INCLUDE_SUBPROCESS_STARTINFO_HPP
#define INCLUDE_SUBPROCESS_STARTINFO_HPP

#include <string>
#include <vector>

namespace Framework::SubProcess {
	class StartInfo {
		std::string command_;
		std::vector<std::string> arguments_;
		std::vector<std::string> environments_;
		bool redirect_standard_output_{ false };
		bool redirect_standard_error_{ false };
		bool use_shell_{ false };

	public:
		StartInfo() = default;
		StartInfo(std::string command);

		std::string &command();
		const std::string &command() const;

		std::vector<std::string> &arguments();
		const std::vector<std::string> &arguments() const;

		std::vector<std::string> &environments();
		const std::vector<std::string> &environments() const;

		bool &redirect_standard_output();
		bool redirect_standard_output() const;

		bool &redirect_standard_error();
		bool redirect_standard_error() const;

		bool &use_shell();
		bool use_shell() const;

		std::string get_command_line() const;

		// nullptr 終端の const char* 配列を vector で返す（メモリ管理は vector が担う）
		std::vector<const char *> c_style_environments() const;
	};
}; // namespace Framework::SubProcess

#endif // INCLUDE_SUBPROCESS_STARTINFO_HPP
