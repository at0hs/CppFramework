#ifndef INCLUDE_TASK_MESSAGEEVENTAGGREGATOR_HPP
#define INCLUDE_TASK_MESSAGEEVENTAGGREGATOR_HPP

#include <cstdint>
#include <functional>
#include <map>
#include <stdexcept>
#include <utility>

#include "Exception/Error.hpp"
#include "Exception/Exception.hpp"

#include "Task/EventRequest.hpp"

namespace Framework::Task {
	using namespace Framework;

	template <typename T = EventRequest<>::Command, typename U = int64_t>
	class MessageEventAggregator {
	public:
		using EventHandler = std::function<bool(const EventRequest<T> &)>;
		using Command = T;
		using State = U;
		static constexpr State kKeepState = static_cast<State>(-1);

		class MessageInfo {
			EventHandler handler_;
			State next_state_;

		public:
			MessageInfo(EventHandler handler, State next_state = kKeepState)
				: handler_(handler),
				  next_state_(next_state) {}

			bool handle(const EventRequest<T> &req) { return handler_(req); }

			State get_next_state() const { return next_state_; }
		};

		using MessageEvent = std::pair<const Command, MessageInfo>;
		using EventMap = std::map<Command, MessageInfo>;

	private:
		EventMap events_;

	public:
		MessageEventAggregator() = default;

		MessageEventAggregator(const EventMap &events) : events_(events) {}

		void add(Command command, EventHandler handler, State next_state = kKeepState) {
			events_.emplace(command, MessageInfo(handler, next_state));
		}

		bool publish(Command command, const EventRequest<T> &req) {
			try {
				auto &info = events_.at(command);
				return info.handle(req);
			} catch (const std::out_of_range &e) {
				throw Exception("Event not found", Error::Code::OutOfRange);
			}
		}

		State get_next_state(Command command) const {
			try {
				return events_.at(command).get_next_state();
			} catch (const std::out_of_range &e) {
				throw Exception("Event not found", Error::Code::OutOfRange);
			}
		}
	};
} // namespace Framework::Task
#endif // INCLUDE_TASK_MESSAGEEVENTAGGREGATOR_HPP
