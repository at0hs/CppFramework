#ifndef INCLUDE_TASK_STATEMENTTASK_HPP
#define INCLUDE_TASK_STATEMENTTASK_HPP

#include <optional>
#include <string>
#include <type_traits>

#include "Task/EventTaskBase.hpp"
#include "Task/StateMachine.hpp"
#include "Task/TaskBase.hpp"

#include "Templates/Property.hpp"

namespace Framework::Task {
	using namespace Framework::Templates;

	template <typename T = StateMachine<>::State, typename U = StateMachine<>::Command,
			  std::enable_if_t<(std::is_integral_v<T> || std::is_enum_v<T>) &&
								   (std::is_integral_v<U> || std::is_enum_v<U>),
							   std::nullptr_t> = nullptr>
	class StatementTask : public EventTaskBase<U, StateMachine<U, T>> {
		using Base = EventTaskBase<U, StateMachine<U, T>>;

	public:
		using State = T;
		using Command = U;

		using SM = StateMachine<Command, State>;
		using StateTable = typename SM::StateTable;
		using StateEvents = typename SM::StateEvents;

		using Events = typename SM::EventAggregator;
		using EventHandler = typename Events::EventHandler;
		static constexpr State kKeepState = Events::kKeepState;

		ReferenceProperty::FunctionSetter<void(State, State)> state_changed;

		// StateTable を直接渡す既存コンストラクタ
		StatementTask(const std::string &name, const StateTable &table, State initial_state)
			: Base(TaskType::Statement, name, table, initial_state),
			  state_changed(this->aggregator().state_changed) {}

		// ビルダー API 用コンストラクタ: on() で遷移を追加し start() で確定する
		StatementTask(const std::string &name, State initial_state)
			: Base(TaskType::Statement, name),
			  state_changed(this->aggregator().state_changed),
			  pending_initial_state_(initial_state) {}

		// ビルダー API: 遷移を1つ登録する（メソッドチェーン可）
		StatementTask &on(State from, Command cmd, EventHandler handler,
						  State next_state = kKeepState) {
			this->aggregator().add(from, cmd, handler, next_state);
			return *this;
		}

		// start() で初期状態を確定してからメッセージポンプを起動する
		void start() override {
			if (pending_initial_state_) {
				this->aggregator().init(*pending_initial_state_);
			}
			Base::start();
		}

		void set_state(State new_state) { this->aggregator().set_state(new_state); }

		State get_state() const { return this->aggregator().get_state(); }

	private:
		std::optional<State> pending_initial_state_;
	};

} // namespace Framework::Task
#endif // INCLUDE_TASK_STATEMENTTASK_HPP
