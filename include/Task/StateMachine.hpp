#ifndef INCLUDE_TASK_STATEMACHINE_HPP
#define INCLUDE_TASK_STATEMACHINE_HPP

#include <atomic>
#include <functional>
#include <map>
#include <stdexcept>
#include <utility>

#include "Exception/Error.hpp"
#include "Exception/Exception.hpp"
#include "Task/EventRequest.hpp"
#include "Task/MessageEventAggregator.hpp"

namespace Framework::Task {

	template <typename T = EventRequest<>::Command, typename U = MessageEventAggregator<>::State>
	class StateMachine {
	public:
		using Command = T;
		using State = U;
		using EventAggregator = MessageEventAggregator<Command, State>;
		using EventHandler = typename EventAggregator::EventHandler;
		using StateTable = std::map<State, EventAggregator>;
		using StateEvents = std::pair<const State, EventAggregator>;

	private:
		static constexpr State kUndefinedState = static_cast<State>(-1);
		class StateInfo {
		public:
			State state{ kUndefinedState };
			EventAggregator *aggregator{ nullptr };
			StateInfo() = default;
			StateInfo(State state, EventAggregator &aggregator)
				: state(state),
				  aggregator(&aggregator) {}
		};
		StateTable table_;
		std::atomic<StateInfo> current_;

	public:
		// テーブルを直接渡す既存コンストラクタ
		StateMachine(const StateTable &table, State initial_state) : table_(table) {
			set_state(initial_state);
		}

		// ビルダー API 用デフォルトコンストラクタ（add() → init() の順で使用）
		StateMachine() = default;

		// ビルダー API: 遷移を1つ登録する
		void add(State from, Command cmd, EventHandler handler,
				 State next_state = EventAggregator::kKeepState) {
			table_[from].add(cmd, handler, next_state);
		}

		// ビルダー API: 初期状態を確定する（start() 前に必ず呼ぶ）
		void init(State initial_state) { set_state(initial_state); }

		void set_state(State new_state) {
			State current_state = current_.load().state;
			try {
				current_.store({ new_state, table_.at(new_state) });
				if (state_changed) {
					state_changed(current_state, new_state);
				}
			} catch (const std::out_of_range &e) {
				throw Exception("State not found", Error::Code::OutOfRange);
			}
		}

		State get_state() const { return current_.load().state; }

		bool publish(Command command, const EventRequest<Command> &req) {
			auto current = current_.load();
			if (!current.aggregator) {
				throw Exception("State machine not initialized", Error::Code::InvalidOperation);
			}
			if (!current.aggregator->publish(command, req)) {
				return  false;
			}
			State next_state = current.aggregator->get_next_state(command);
			if ((next_state != EventAggregator::kKeepState) && (current_.load().state != next_state)) {
				set_state(next_state);
			}
			return true;
		}

		std::function<void(State, State)> state_changed;
	};

} // namespace Framework::Task
#endif // INCLUDE_TASK_STATEMACHINE_HPP
