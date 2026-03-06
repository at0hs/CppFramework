#ifndef INCLUDE_TASK_BACKGROUNDWORKER_HPP
#define INCLUDE_TASK_BACKGROUNDWORKER_HPP

#include <any>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include "Exception/Error.hpp"
#include "Task/TaskBase.hpp"
#include "Templates/Property.hpp"

namespace Framework::Task {
	using namespace Framework::Templates;

	class BackGroundWorker : public TaskBase {
	public:
		class DoTaskEventArgs {
			bool cancel_{ false };
			std::any result_;

		public:
			DoTaskEventArgs() = default;

			void set_cancel(bool cancel) { cancel_ = cancel; }

			bool get_cancel() const { return cancel_; }

			void set_result(const std::any &result) { result_ = result; }

			void set_result(std::any &&result) { result_ = std::move(result); }

			const std::any &get_result() const { return result_; }
		};

		class TaskCompletedEventArgs {
			bool cancelled_{ false };
			const std::any *result_{ nullptr };
			Framework::Error::Code error_{ Framework::Error::Code::Unknown };

		public:
			TaskCompletedEventArgs(bool cancelled, const std::any &result,
								   Framework::Error::Code error)
				: cancelled_(cancelled),
				  result_(&result),
				  error_(error) {}

			bool cancelled() const { return cancelled_; }

			Framework::Error::Code error_code() const { return error_; }

			template <typename T = std::any>
			const auto &result() const {
				return std::any_cast<const T &>(*result_);
			}
		};

		using Progress = uint8_t;

		class ProgressChangedEventArgs {
			Progress progress_{ 0 };

		public:
			explicit ProgressChangedEventArgs(Progress progress) : progress_(progress) {}

			Progress progress_percent() const { return progress_; }
		};

		using Task = std::function<void(BackGroundWorker &, DoTaskEventArgs &)>;
		using TaskCompletedEventHandler =
			std::function<void(BackGroundWorker &, TaskCompletedEventArgs &)>;
		using ProgressChangedEventHandler =
			std::function<void(BackGroundWorker &, ProgressChangedEventArgs &)>;

	private:
		std::thread thread_;
		Task task_;
		std::mutex mutex_;
		std::condition_variable condition_;
		std::atomic<bool> running_{ false };
		bool stop_{ false };
		std::atomic<bool> cancellation_pending_{ false };
		Progress progress_{ 0 };

		TaskCompletedEventHandler task_completed_;
		ProgressChangedEventHandler progress_changed_;

	public:
		BackGroundWorker(const std::string &name, Task task);
		virtual ~BackGroundWorker();

		BackGroundWorker(const BackGroundWorker &) = delete;
		BackGroundWorker &operator=(const BackGroundWorker &) = delete;
		BackGroundWorker(BackGroundWorker &&) = delete;
		BackGroundWorker &operator=(BackGroundWorker &&) = delete;

		void run_task_async();
		bool cancellation_pending() const;
		void cancel_async();
		void reports_progress(Progress percent);
		bool is_busy() const;

		ReferenceProperty::FunctionSetter<TaskCompletedEventHandler> task_completed{
			task_completed_
		};
		ReferenceProperty::FunctionSetter<ProgressChangedEventHandler> progress_changed{
			progress_changed_
		};

	private:
		void sleep();
		void on_do_task();
		void on_task_completed(DoTaskEventArgs &do_task_event_args, Framework::Error::Code error);
	};
} // namespace Framework::Task
#endif // INCLUDE_TASK_BACKGROUNDWORKER_HPP
