#include "Sync/EventFlag.hpp"
#include <chrono>
#include <mutex>

namespace EFlag {

	bool EventFlag::match(Flag pattern, MatchMode mode) const {
		if (mode == MatchMode::AND) {
			return pattern == pattern_.load();
		}
		return (pattern & pattern_.load()) != 0;
	}

	void EventFlag::wait(Flag pattern, MatchMode mode) {
		std::unique_lock<std::mutex> lock(mutex_);
		condition_.wait(lock, [&] { return match(pattern, mode); });
	}

	bool EventFlag::timed_wait(Flag pattern, MatchMode mode, std::chrono::milliseconds millisec) {
		std::unique_lock<std::mutex> lock(mutex_);
		return condition_.wait_for(lock, millisec, [&] { return match(pattern, mode); });
	}

	void EventFlag::set(Flag pattern) {
		{
			std::scoped_lock const lock(mutex_);
			pattern_ |= pattern;
		}
		condition_.notify_all();
	}

	Flag EventFlag::get() const {
		return pattern_.load();
	}

} // namespace EFlag
