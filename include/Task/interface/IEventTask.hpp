#ifndef INCLUDE_TASK_INTERFACE_IMESSAGETASK_HPP
#define INCLUDE_TASK_INTERFACE_IMESSAGETASK_HPP

#include <chrono>

#include "Task/EventRequest.hpp"

namespace Framework::Task {
	template <typename T = EventRequest<>::Command>
	class IEventTask {
	public:
		static constexpr auto kWaitForever = std::chrono::milliseconds::zero();

		// IEventTask(TaskType type, const std::string &name) : TaskBase(type, name) {}

		virtual void start() = 0;
		virtual void stop() = 0;

		virtual void send_event(EventRequest<T> &&message) = 0;
		virtual void send_event(const EventRequest<T> &message) = 0;

		virtual bool rpc_event(EventRequest<T> &&message,
							   std::chrono::milliseconds timeout_msec = kWaitForever) = 0;
		virtual bool rpc_event(const EventRequest<T> &message,
							   std::chrono::milliseconds timeout_msec = kWaitForever) = 0;
	};
} // namespace Framework::Task

#endif // INCLUDE_TASK_INTERFACE_IMESSAGETASK_HPP
