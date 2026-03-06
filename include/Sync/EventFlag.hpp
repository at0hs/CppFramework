#ifndef INCLUDE_SYNC_EVENTFLAG_HPP
#define INCLUDE_SYNC_EVENTFLAG_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace EFlag {
	using Flag = std::uint64_t;
	enum class MatchMode : std::uint8_t {
		AND,
		OR,
	};

	class EventFlag {
		std::atomic<Flag> pattern_{ 0 };
		std::mutex mutex_;
		std::condition_variable condition_;
		[[nodiscard]] bool match(Flag pattern, MatchMode mode) const;

	public:
		void wait(Flag pattern, MatchMode mode);
		bool timed_wait(Flag pattern, MatchMode mode, std::chrono::milliseconds millisec);
		void set(Flag pattern);
		Flag get() const;
	};
} // namespace EFlag

#endif // INCLUDE_SYNC_EVENTFLAG_HPP
