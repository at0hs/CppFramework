#ifndef INCLUDE_TIMER_HPP
#define INCLUDE_TIMER_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class Timer {
	std::chrono::milliseconds time_;
	std::atomic<bool> restart_{ false };
	std::atomic<bool> active_{ false };
	std::thread thread_;
	std::mutex handlers_mutex_;
	std::vector<std::function<void()>> event_handlers_;

public:
	explicit Timer(std::chrono::milliseconds time) : time_(time) {}

	~Timer();

	Timer(const Timer &) = delete;
	Timer &operator=(const Timer &) = delete;
	Timer(Timer &&) = delete;
	Timer &operator=(Timer &&) = delete;

	void auto_restart(bool val) { restart_ = val; }

	void add_listener(const std::function<void()> &f) {
		std::lock_guard lock(handlers_mutex_);
		event_handlers_.emplace_back(f);
	}

	void start();
	void stop();
	void wait_for_stop();
};
#endif // INCLUDE_TIMER_HPP
