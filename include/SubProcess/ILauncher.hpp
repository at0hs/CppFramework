#ifndef INCLUDE_SUBPROCESS_ILAUNCHER_HPP
#define INCLUDE_SUBPROCESS_ILAUNCHER_HPP

#include "StartInfo.hpp"

namespace Framework::SubProcess {
	class ILauncher {
	public:
		virtual ~ILauncher() = default;
		ILauncher(const ILauncher &) = delete;
		ILauncher &operator=(const ILauncher &) = delete;
		ILauncher(ILauncher &&) = delete;
		ILauncher &operator=(ILauncher &&) = delete;

		// 子プロセス内で呼び出す。成功時は返らず、失敗時は負の値を返す。
		virtual int launch(const StartInfo &info) = 0;

	protected:
		ILauncher() = default;
	};
} // namespace Framework::SubProcess

#endif // INCLUDE_SUBPROCESS_ILAUNCHER_HPP
