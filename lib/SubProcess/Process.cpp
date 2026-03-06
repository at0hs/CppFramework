#include "SubProcess/Process.hpp"

#include <array>
#include <csignal>
#include <sys/resource.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "Exception/SystemException.hpp"
#include "SubProcess/ExecFamily.hpp"
#include "SubProcess/Shell.hpp"

namespace Framework::SubProcess {

	namespace {
		// パイプの fd を読み切って std::string に格納する
		std::string read_pipe(int fd) {
			if (fd == -1) {
				return {};
			}
			std::string result;
			std::array<char, 4096> buf{};
			ssize_t n = 0;
			while ((n = ::read(fd, buf.data(), buf.size())) > 0) {
				result.append(buf.data(), static_cast<size_t>(n));
			}
			return result;
		}
	} // namespace

	// --- コンストラクタ / デストラクタ ---

	Process::Process(StartInfo start_info) : start_info_(std::move(start_info)) {}

	Process::Process(Process &&other) noexcept
		: start_info_(std::move(other.start_info_)),
		  id_(other.id_),
		  has_exited_(other.has_exited_.load()) {
		other.id_ = -1;
	}

	Process &Process::operator=(Process &&other) noexcept {
		if (this != &other) {
			start_info_ = std::move(other.start_info_);
			id_ = other.id_;
			has_exited_.store(other.has_exited_.load());
			other.id_ = -1;
		}
		return *this;
	}

	Process::~Process() {
		// 子プロセスが残っていれば kill して回収
		if (id_ > 0 && !has_exited_.load()) {
			::kill(id_, SIGKILL);
			int status{ 0 };
			::waitpid(id_, &status, 0);
		}
	}

	// --- インスタンスメソッド ---

	std::future<ProcessResult> Process::start_async() {
		auto start_time = std::chrono::system_clock::now();

		// リダイレクト用パイプを作成
		std::array<int, 2> stdout_pipe{ -1, -1 };
		std::array<int, 2> stderr_pipe{ -1, -1 };

		if (start_info_.redirect_standard_output()) {
			if (::pipe(stdout_pipe.data()) != 0) {
				throw Framework::SystemException("pipe() failed for stdout", errno);
			}
		}
		if (start_info_.redirect_standard_error()) {
			if (::pipe(stderr_pipe.data()) != 0) {
				const auto saved_errno = errno;
				if (stdout_pipe[0] != -1) {
					::close(stdout_pipe[0]);
					::close(stdout_pipe[1]);
				}
				throw Framework::SystemException("pipe() failed for stderr", saved_errno);
			}
		}

		pid_t pid = ::fork();
		if (pid < 0) {
			// fork 失敗
			const auto saved_errno = errno;
			if (stdout_pipe[0] != -1) {
				::close(stdout_pipe[0]);
				::close(stdout_pipe[1]);
			}
			if (stderr_pipe[0] != -1) {
				::close(stderr_pipe[0]);
				::close(stderr_pipe[1]);
			}
			throw Framework::SystemException("fork() failed", saved_errno);
		}

		if (pid == 0) {
			// 子プロセス: 読み込み端を閉じる
			if (stdout_pipe[0] != -1) {
				::close(stdout_pipe[0]);
			}
			if (stderr_pipe[0] != -1) {
				::close(stderr_pipe[0]);
			}
			int ret = child_process(&start_info_, stdout_pipe[1], stderr_pipe[1]);
			::_exit(ret);
		}

		// 親プロセス: 書き込み端を閉じる
		if (stdout_pipe[1] != -1) {
			::close(stdout_pipe[1]);
		}
		if (stderr_pipe[1] != -1) {
			::close(stderr_pipe[1]);
		}

		id_ = pid;

		// スレッドで待機し、promise 経由で結果を返す
		std::promise<ProcessResult> promise;
		auto future = promise.get_future();

		std::thread([p = std::move(promise), pid, start_time, stdout_fd = stdout_pipe[0],
					 stderr_fd = stderr_pipe[0], this]() mutable {
			auto result = collect_result(pid, start_time, stdout_fd, stderr_fd);
			has_exited_.store(true);
			p.set_value(std::move(result));
		}).detach();

		return future;
	}

	ProcessResult Process::start() {
		return start_async().get();
	}

	void Process::kill() {
		if (id_ <= 0 || has_exited_.load()) {
			return;
		}
		// まず SIGTERM を送り、プロセスが終了する機会を与える
		::kill(id_, SIGTERM);

		// 最大 500ms 待機
		static constexpr auto grace_period = std::chrono::milliseconds(500);
		static constexpr auto interval = std::chrono::milliseconds(10);
		auto deadline = std::chrono::steady_clock::now() + grace_period;

		while (std::chrono::steady_clock::now() < deadline) {
			int status{ 0 };
			pid_t ret = ::waitpid(id_, &status, WNOHANG);
			if (ret == id_) {
				has_exited_.store(true);
				return;
			}
			std::this_thread::sleep_for(interval);
		}

		// まだ生きていれば SIGKILL
		if (!has_exited_.load()) {
			::kill(id_, SIGKILL);
			int status{ 0 };
			::waitpid(id_, &status, 0);
			has_exited_.store(true);
		}
	}

	// --- プライベート ---

	int Process::child_process(const StartInfo *info, int stdout_fd, int stderr_fd) {
		// stdout リダイレクト
		if (stdout_fd != -1) {
			if (::dup2(stdout_fd, STDOUT_FILENO) == -1) {
				return kProcessFailed;
			}
			::close(stdout_fd);
		}
		// stderr リダイレクト
		if (stderr_fd != -1) {
			if (::dup2(stderr_fd, STDERR_FILENO) == -1) {
				return kProcessFailed;
			}
			::close(stderr_fd);
		}

		// ILauncher 経由でコマンドを exec
		if (info->use_shell()) {
			Shell launcher;
			return launcher.launch(*info);
		}
		ExecFamily launcher;
		return launcher.launch(*info);
	}

	ProcessResult Process::collect_result(pid_t pid,
										  std::chrono::system_clock::time_point start_time,
										  int stdout_read_fd, int stderr_read_fd) {
		// パイプから出力を読み切る（子プロセス側の書き込み端は既に閉じ済み）
		std::string stdout_str = read_pipe(stdout_read_fd);
		std::string stderr_str = read_pipe(stderr_read_fd);

		if (stdout_read_fd != -1) {
			::close(stdout_read_fd);
		}
		if (stderr_read_fd != -1) {
			::close(stderr_read_fd);
		}

		// プロセス終了を待機して rusage を取得
		int status{ -1 };
		rusage usage{};
		::wait4(pid, &status, 0, &usage);

		auto exit_time = std::chrono::system_clock::now();

		ProcessResult result;
		result.has_exited_ = true;
		result.exit_code_ = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
		result.usage_ = usage;
		result.start_time_ = start_time;
		result.exit_time_ = exit_time;
		result.standard_output_ = std::move(stdout_str);
		result.standard_error_ = std::move(stderr_str);
		return result;
	}

} // namespace Framework::SubProcess
