#ifndef INCLUDE_SUBPROCESS_PROCESSRESULT_HPP
#define INCLUDE_SUBPROCESS_PROCESSRESULT_HPP

#include <chrono>
#include <string>
#include <sys/resource.h>

namespace Framework::SubProcess {
	class Process;

	// プロセス実行結果を保持するクラス。
	// Process クラスからのみ構築可能（friend 経由）。
	class ProcessResult {
		using Clock = std::chrono::system_clock;
		using TimePoint = Clock::time_point;
		using Duration = std::chrono::microseconds;

		int32_t exit_code_{ -1 };
		bool has_exited_{ false };
		rusage usage_{};
		TimePoint start_time_;
		TimePoint exit_time_;
		std::string standard_output_;
		std::string standard_error_;

	public:
		int32_t exit_code() const { return exit_code_; }

		bool has_exited() const { return has_exited_; }

		TimePoint start_time() const { return start_time_; }

		TimePoint exit_time() const { return exit_time_; }

		// ユーザー時間 + カーネル時間の合計 CPU 使用時間
		Duration total_processor_time() const {
			auto to_us = [](const timeval &tv) -> int64_t {
				return (static_cast<int64_t>(tv.tv_sec) * 1'000'000) + tv.tv_usec;
			};
			return Duration{ to_us(usage_.ru_utime) + to_us(usage_.ru_stime) };
		}

		// ユーザーモードでの CPU 使用時間
		Duration user_processor_time() const {
			auto to_us = [](const timeval &tv) -> int64_t {
				return (static_cast<int64_t>(tv.tv_sec) * 1'000'000) + tv.tv_usec;
			};
			return Duration{ to_us(usage_.ru_utime) };
		}

		const std::string &standard_output() const { return standard_output_; }

		const std::string &standard_error() const { return standard_error_; }

	private:
		ProcessResult() = default;
		friend class Process;
	};
} // namespace Framework::SubProcess

#endif // INCLUDE_SUBPROCESS_PROCESSRESULT_HPP
