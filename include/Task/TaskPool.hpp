#ifndef INCLUDE_TASK_TASKPOOL_HPP
#define INCLUDE_TASK_TASKPOOL_HPP
#include <sched.h>
#include <unistd.h>
#ifndef __GNU_SOURCE
#define __GNU_SOURCE
#include <pthread.h>
#undef __GNU_SOURCE
#endif
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <vector>

#include "Task/TaskBase.hpp"

namespace Framework::Task {

	class TaskPool : public TaskBase {
		using Task = std::function<void()>;

	private:
		std::deque<Task> tasks_;
		std::condition_variable condition_;
		std::mutex mutex_;
		std::vector<std::thread> workers_;
		std::atomic<bool> stop_{ false };
		size_t concurrency_{ 0 };
		std::atomic<size_t> running_tasks_{ 0 };

	public:
		TaskPool(const std::string &name, size_t concurrency = TaskPool::get_concurrency());
		~TaskPool();

		TaskPool(const TaskPool &) = delete;
		TaskPool &operator=(const TaskPool &) = delete;
		TaskPool(TaskPool &&) = delete;
		TaskPool &operator=(TaskPool &&) = delete;

		void enqueue(Task task);
		void stop();
		size_t concurrency() const;
		size_t count_waiting_tasks();
		size_t count_running_tasks();
		void clear_waiting_tasks();
		bool is_empty();

	private:
		void spawn_workers();
		Task wait_for_new_task();
		static size_t get_concurrency();
	};
} // namespace Framework::Task

#endif // INCLUDE_TASK_TASKPOOL_HPP
