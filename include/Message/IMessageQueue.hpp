#ifndef INCLUDE_MESSAGE_IMESSAGEQUEUE_HPP
#define INCLUDE_MESSAGE_IMESSAGEQUEUE_HPP

#include <chrono>
#include <cstddef>
#include <utility>

namespace Framework::Message {
	template <class T>
	class IMessageQueue {
	public:
		IMessageQueue() = default;
		virtual ~IMessageQueue() = default;
		IMessageQueue(const IMessageQueue &) = default;
		IMessageQueue &operator=(const IMessageQueue &) = default;
		IMessageQueue(IMessageQueue &&) = default;
		IMessageQueue &operator=(IMessageQueue &&) = default;

		virtual void send(const T &message) = 0;
		virtual void send(T &&message) = 0;
		virtual T receive() = 0;
		virtual std::pair<bool, T> timed_receive(std::chrono::milliseconds milli_sec) = 0;
		virtual bool is_empty() const = 0;
		virtual void clear() = 0;
		virtual std::size_t num_message() const = 0;
	};
} // namespace Framework::Message

#endif // INCLUDE_MESSAGE_IMESSAGEQUEUE_HPP
