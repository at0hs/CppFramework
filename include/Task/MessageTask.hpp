#ifndef INCLUDE_TASK_MESSAGETASK_HPP
#define INCLUDE_TASK_MESSAGETASK_HPP

#include <string>
#include <type_traits>

#include "Task/EventTaskBase.hpp"
#include "Task/MessageEventAggregator.hpp"
#include "Task/TaskBase.hpp"

namespace Framework::Task {

	template <typename CommandType = MessageEventAggregator<>::Command>
		requires(std::is_integral_v<CommandType> || std::is_enum_v<CommandType>)
	class MessageTask : public EventTaskBase<CommandType, MessageEventAggregator<CommandType>> {
		using Base = EventTaskBase<CommandType, MessageEventAggregator<CommandType>>;

	public:
		using EventAggregator = MessageEventAggregator<CommandType>;
		using EventMap = typename EventAggregator::EventMap;

		MessageTask(const std::string &name, const EventMap &events)
			: Base(TaskType::Message, name, events) {}
	};

} // namespace Framework::Task

#endif // INCLUDE_TASK_MESSAGETASK_HPP
