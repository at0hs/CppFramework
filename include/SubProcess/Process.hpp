#ifndef INCLUDE_SUBPROCESS_PROCESS_HPP
#define INCLUDE_SUBPROCESS_PROCESS_HPP

#include <atomic>
#include <future>
#include <unistd.h>

#include "ProcessResult.hpp"
#include "StartInfo.hpp"

namespace Framework::SubProcess {
	class Process {
	public:
		static constexpr int kProcessFailed = -255;

	private:
		StartInfo start_info_;
		pid_t id_{ -1 };
		std::atomic<bool> has_exited_{ false };

	public:
		Process() = default;
		explicit Process(StartInfo start_info);

		// コピー不可（プロセスハンドルは一意）
		Process(const Process &) = delete;
		Process &operator=(const Process &) = delete;

		// ムーブ可
		Process(Process &&other) noexcept;
		Process &operator=(Process &&other) noexcept;

		~Process();

		// --- インスタンスメソッド ---

		// 非同期でプロセスを起動し、future<ProcessResult> を返す
		std::future<ProcessResult> start_async();

		// プロセスを起動して終了まで待機し、ProcessResult を返す
		ProcessResult start();

		// プロセスを強制終了する（SIGTERM 後に SIGKILL）
		void kill();

		// --- プロパティ ---

		pid_t id() const { return id_; }

		bool has_exited() const { return has_exited_.load(); }

	private:
		// 子プロセス内で呼び出す。ILauncher 経由でコマンドを exec する。
		// stdout_fd / stderr_fd はパイプの書き込み端（-1 はリダイレクト無効）。
		static int child_process(const StartInfo *info, int stdout_fd, int stderr_fd);

		// 親プロセス側でパイプを読み込みながら wait4() し、ProcessResult を構築する
		static ProcessResult collect_result(pid_t pid,
											std::chrono::system_clock::time_point start_time,
											int stdout_read_fd, int stderr_read_fd);
	};
} // namespace Framework::SubProcess

#endif // INCLUDE_SUBPROCESS_PROCESS_HPP
