#include "SubProcess/ExecFamily.hpp"

#include <unistd.h>

namespace Framework::SubProcess {
	int ExecFamily::launch(const StartInfo &info) {
		// ローカルコピーを作成することで char* (非 const) が得られる（C++17 以降）
		std::string cmd = info.command();
		std::vector<std::string> args = info.arguments();

		std::vector<char *> argv;
		argv.reserve(args.size() + 2);
		argv.push_back(cmd.data());
		for (auto &arg : args) {
			argv.push_back(arg.data());
		}
		argv.push_back(nullptr);

		const auto &envs = info.environments();
		if (envs.empty()) {
			// 親プロセスの環境変数を継承
			execvp(cmd.data(), argv.data());
		} else {
			// 指定した環境変数で実行（execvpe は GNU 拡張）
			std::vector<std::string> env_copies = envs;
			std::vector<char *> envp;
			envp.reserve(env_copies.size() + 1);
			for (auto &e : env_copies) {
				envp.push_back(e.data());
			}
			envp.push_back(nullptr);
			execvpe(cmd.data(), argv.data(), envp.data());
		}

		// exec が返った場合は失敗
		return -1;
	}
} // namespace Framework::SubProcess
