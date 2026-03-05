#ifndef INCLUDE_TASK_TASKBASE_HPP
#define INCLUDE_TASK_TASKBASE_HPP

#include <thread>

namespace Framework::Task {

	enum class TaskType : int8_t {
		Unknown = 0,
		Message,
		Statement,
		RealTime,
		BackGround,
		TaskPool,
	};

	struct TaskInformation {
		std::string name;
		TaskType type;
		std::thread::id thread_id;
	};

	class TaskBase {
	protected:
		std::string name_;
		TaskType type_{ TaskType::Unknown };
		std::thread thread_;

	public:
		TaskBase(TaskType type, std::string name);
		virtual ~TaskBase() = default;

		TaskBase(const TaskBase &) = delete;
		TaskBase &operator=(const TaskBase &) = delete;
		TaskBase(TaskBase &&) = delete;
		TaskBase &operator=(TaskBase &&) = delete;

		virtual TaskInformation get_task_information();
	};
} // namespace Framework::Task

#endif // INCLUDE_TASK_TASKBASE_HPP
