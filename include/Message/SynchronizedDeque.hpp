#ifndef INCLUDE_MESSAGE_SYNCHRONIZEDDEQUE_HPP
#define INCLUDE_MESSAGE_SYNCHRONIZEDDEQUE_HPP

#include "IMessageQueue.hpp"
#include <condition_variable>
#include <deque>
#include <mutex>

namespace Framework::Message {
	template <typename T>
	class SynchronizedDeque : public IMessageQueue<T> {
		static constexpr auto kWaitForever = std::chrono::milliseconds::zero();
		std::mutex mutex_;
		std::condition_variable condition_;
		std::deque<T> queue_{};

		T get_front() {
			T buffer = std::move(queue_.front());
			queue_.pop_front();
			return buffer;
		}

		bool is_not_empty() const { return !queue_.empty(); }

	public:
		SynchronizedDeque() = default;

		void send(const T &message) override {
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push_back(message);
			condition_.notify_all();
		}

		void send(T &&message) override {
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push_back(std::move(message));
			condition_.notify_all();
		}

		T receive() override {
			std::unique_lock<std::mutex> lock(mutex_);
			condition_.wait(lock, [this] { return is_not_empty(); });
			return get_front();
		}

		std::pair<bool, T> timed_receive(const std::chrono::milliseconds milli_seconds) override {
			std::unique_lock<std::mutex> lock(mutex_);
			if (condition_.wait_for(lock, milli_seconds, [this] { return is_not_empty(); })) {
				return { true, get_front() };
			}
			return { false, T{} };
		}

		void clear() override {
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.clear();
		}

		bool is_empty() override {
			std::lock_guard<std::mutex> lock(mutex_);
			return queue_.empty();
		}

		std::size_t num_message() override {
			std::lock_guard<std::mutex> lock(mutex_);
			return queue_.size();
		}
	};
} // namespace Framework::Message

#endif // INCLUDE_MESSAGE_SYNCHRONIZEDDEQUE_HPP
