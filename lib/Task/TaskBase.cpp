#include "Task/TaskBase.hpp"

namespace Framework::Task {

	TaskBase::TaskBase(TaskType type, std::string name)
		: name_(std::move(name)),
		  type_{ type } {}

	TaskInformation TaskBase::get_task_information() {
		return { .name = name_, .type = type_, .thread_id = thread_.get_id() };
	}

} // namespace Framework::Task
