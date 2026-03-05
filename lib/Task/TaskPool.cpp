#include "Task/TaskPool.hpp"
#include "Task/TaskBase.hpp"
#include <cstddef>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include <string>
#include <utility>

namespace Framework::Task {

	TaskPool::TaskPool(const std::string &name, size_t concurrency)
		: TaskBase(TaskType::TaskPool, name),
		  concurrency_{ concurrency } {
		spawn_workers();
	}

	TaskPool::~TaskPool() {
		stop();
	}

	void TaskPool::enqueue(Task task) {
		std::scoped_lock const lock(mutex_);
		tasks_.emplace_back(std::move(task));
		condition_.notify_all();
	}

	void TaskPool::stop() {
		stop_ = true;
		condition_.notify_all();
		for (auto &worker : workers_) {
			if (worker.joinable()) {
				worker.join();
			}
		}
		workers_.clear();
	}

	size_t TaskPool::concurrency() const {
		return concurrency_;
	}

	size_t TaskPool::count_waiting_tasks() {
		std::scoped_lock const lock(mutex_);
		return tasks_.size();
	}

	size_t TaskPool::count_running_tasks() {
		return running_tasks_;
	}

	void TaskPool::clear_waiting_tasks() {
		std::scoped_lock const lock(mutex_);
		tasks_.clear();
	}

	bool TaskPool::is_empty() {
		std::scoped_lock const lock(mutex_);
		return tasks_.empty();
	}

	void TaskPool::spawn_workers() {
		for (size_t i = 0; i < concurrency_; i++) {
			workers_.emplace_back([this] {
				while (true) {
					auto task = wait_for_new_task();
					if (stop_ && !task) {
						return;
					}
					if (task) {
						running_tasks_++;
						task();
						running_tasks_--;
					}
				}
			});
		}
	}

	TaskPool::Task TaskPool::wait_for_new_task() {
		std::unique_lock<std::mutex> lock(mutex_);
		condition_.wait(lock, [this] { return !tasks_.empty() || stop_.load(); });
		if (tasks_.empty()) {
			return {};
		}
		auto task = std::move(tasks_.front());
		tasks_.pop_front();
		return task;
	}

	size_t TaskPool::get_concurrency() {
		cpu_set_t cpu_set{ 0 };
		CPU_ZERO(&cpu_set);
		pthread_getaffinity_np(pthread_self(), sizeof(cpu_set), &cpu_set);
		return static_cast<size_t>(CPU_COUNT(&cpu_set));
	}

} // namespace Framework::Task
