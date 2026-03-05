#ifndef INCLUDE_SUBPROCESS_EXECFAMILY_HPP
#define INCLUDE_SUBPROCESS_EXECFAMILY_HPP

#include "ILauncher.hpp"

namespace Framework::SubProcess {
	// execvp を使って直接コマンドを実行するランチャー。
	// use_shell() == false のときに使用する。
	class ExecFamily : public ILauncher {
	public:
		ExecFamily() = default;
		int launch(const StartInfo &info) override;
	};
} // namespace Framework::SubProcess

#endif // INCLUDE_SUBPROCESS_EXECFAMILY_HPP
