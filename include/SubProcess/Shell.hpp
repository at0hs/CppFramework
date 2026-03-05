#ifndef INCLUDE_SUBPROCESS_SHELL_HPP
#define INCLUDE_SUBPROCESS_SHELL_HPP

#include "ILauncher.hpp"

namespace Framework::SubProcess {
	// /bin/sh -c を使ってコマンドをシェル経由で実行するランチャー。
	// use_shell() == true のときに使用する。
	class Shell : public ILauncher {
	public:
		Shell() = default;
		int launch(const StartInfo &info) override;
	};
} // namespace Framework::SubProcess

#endif // INCLUDE_SUBPROCESS_SHELL_HPP
