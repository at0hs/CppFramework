#include "Task/BackGroundWorker.hpp"
#include "Exception/Error.hpp"
#include "Exception/Exception.hpp"
#include "Task/TaskBase.hpp"
#include <mutex>
#include <string>
#include <utility>

namespace Framework::Task {

	BackGroundWorker::BackGroundWorker(const std::string &name, Task task)
		: TaskBase(TaskType::BackGround, name),
		  task_(std::move(task)) {
		thread_ = std::thread([this] {
			while (true) {
				sleep();
				if (stop_) {
					break;
				}
				on_do_task();
			}
		});
	}

	BackGroundWorker::~BackGroundWorker() {
		{
			std::scoped_lock const lock(mutex_);
			stop_ = true;
			condition_.notify_all();
		}
		if (thread_.joinable()) {
			thread_.join();
		}
	}

	void BackGroundWorker::run_task_async() {
		std::scoped_lock const lock(mutex_);
		running_ = true;
		condition_.notify_all();
	}

	bool BackGroundWorker::cancellation_pending() const  {
		return cancellation_pending_;
	}

	void BackGroundWorker::cancel_async() {
		cancellation_pending_ = true;
	}

	void BackGroundWorker::reports_progress(Progress percent) {
		ProgressChangedEventArgs args(percent);
		if (progress_changed_) {
			progress_changed_(*this, args);
		}
	}

	bool BackGroundWorker::is_busy() {
		return running_;
	}

	void BackGroundWorker::sleep() {
		std::unique_lock<std::mutex> lock(mutex_);
		condition_.wait(lock, [this] { return running_.load() || stop_; });
	}

	void BackGroundWorker::on_do_task() {
		DoTaskEventArgs args;
		Framework::Error::Code error = Framework::Error::Code::Success;
		try {
			task_(*this, args);
		} catch (const Framework::Exception &e) {
			error = e.get_code();
		}
		on_task_completed(args, error);
	}

	void BackGroundWorker::on_task_completed(DoTaskEventArgs &do_task_event_args,
											  Framework::Error::Code error) {
		if (!task_completed_) {
			running_ = false;
			cancellation_pending_ = false;
			progress_ = 0;
			return;
		}
		TaskCompletedEventArgs args{ do_task_event_args.get_cancel(), do_task_event_args.get_result(),
									 error };
		task_completed_(*this, args);

		cancellation_pending_ = false;
		running_ = false;
		progress_ = 0;
	}

} // namespace Framework::Task
