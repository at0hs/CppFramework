#include "SubProcess/Shell.hpp"

#include <unistd.h>

namespace Framework::SubProcess {
	int Shell::launch(const StartInfo &info) {
		std::string cmd = info.get_command_line();

		const auto &envs = info.environments();
		if (envs.empty()) {
			// 親プロセスの環境変数を継承してシェル実行
			execl("/bin/sh", "sh", "-c", cmd.c_str(), static_cast<char *>(nullptr));
		} else {
			// 指定した環境変数でシェル実行
			std::vector<std::string> env_copies = envs;
			std::vector<char *> envp;
			envp.reserve(env_copies.size() + 1);
			for (auto &e : env_copies) {
				envp.push_back(e.data());
			}
			envp.push_back(nullptr);
			execle("/bin/sh", "sh", "-c", cmd.c_str(), static_cast<char *>(nullptr), envp.data());
		}

		// exec が返った場合は失敗
		return -1;
	}
} // namespace Framework::SubProcess
