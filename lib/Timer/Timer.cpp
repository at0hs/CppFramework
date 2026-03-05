#include "Timer.hpp"
#include <functional>
#include <thread>
#include <vector>

Timer::~Timer() {
	wait_for_stop();
}

void Timer::start() {
	if (active_) {
		return;
	}
	if (thread_.joinable()) {
		thread_.join();
	}
	active_ = true;
	thread_ = std::thread{ [&] {
		std::vector<std::function<void()>> const handlers{ this->event_handlers_ };
		while (true) {
			std::this_thread::sleep_for(this->time_);
			if (!this->active_) {
				break;
			}
			for (const auto &f : handlers) {
				std::invoke(f);
			}
			if (!this->restart_) {
				break;
			}
		};
		this->thread_.detach();
	} };
}

void Timer::stop() {
	if (active_) {
		active_ = false;
	}
}

void Timer::wait_for_stop() {
	stop();
	if (thread_.joinable()) {
		thread_.join();
	}
}
