#include "StatementTask.hpp"

#include <thread>

#include "Task/StatementTask.hpp"

using namespace Framework::Task;

namespace {
	enum class TestState: uint8_t { IDLE, RUNNING, STOPPED };
	enum class TestCommand: uint8_t { START, STOP };

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	EventRequest<TestCommand> last_request{ "", static_cast<TestCommand>(-1) };

	inline bool event_handler(const EventRequest<TestCommand> &req) {
		last_request = req;
		return true;
	}

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	const StatementTask<TestState, TestCommand>::Events kStoppedEvents{ {
		{ TestCommand::START, { event_handler, TestState::IDLE } },
		{ TestCommand::STOP, { event_handler, StatementTask<TestState, TestCommand>::kKeepState } },
	} };

	const StatementTask<TestState, TestCommand>::Events kIdleEvents = { {
		{ TestCommand::START, { event_handler, TestState::RUNNING } },
		{ TestCommand::STOP, { event_handler, TestState::STOPPED } },
	} };

	const StatementTask<TestState, TestCommand>::Events kRunningEvents = { {
		{ TestCommand::START,
		  { event_handler, StatementTask<TestState, TestCommand>::kKeepState } },
		{ TestCommand::STOP, { event_handler, TestState::IDLE } },
	} };

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	StatementTask<TestState, TestCommand>::StateTable table{
		{ TestState::IDLE, kIdleEvents },
		{ TestState::RUNNING, kRunningEvents },
		{ TestState::STOPPED, kStoppedEvents },
	};
} // namespace StatementTaskUnitTest

void StatementTaskTest::SetUp() {
	last_request = EventRequest<TestCommand>{ "", static_cast<TestCommand>(-1) };
}

TEST_F(StatementTaskTest, NormalSequence) {
	StatementTask<TestState, TestCommand> task("test", table, TestState::STOPPED);
	EventRequest<TestCommand> start_request{ "", TestCommand::START };
	EventRequest<TestCommand> stop_request{ "", TestCommand::STOP };

	EXPECT_EQ(static_cast<int>(TestState::STOPPED), static_cast<int>(task.get_state()));

	task.send_event(start_request);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_EQ(static_cast<int>(TestState::IDLE), static_cast<int>(task.get_state()));

	task.send_event(start_request);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_EQ(static_cast<int>(TestState::RUNNING), static_cast<int>(task.get_state()));

	task.send_event(stop_request);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_EQ(static_cast<int>(TestState::IDLE), static_cast<int>(task.get_state()));

	task.send_event(stop_request);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_EQ(static_cast<int>(TestState::STOPPED), static_cast<int>(task.get_state()));
}

TEST_F(StatementTaskTest, RpcSequence) {
	StatementTask<TestState, TestCommand> task("test", table, TestState::STOPPED);
	EventRequest<TestCommand> start_request{ "", TestCommand::START };
	EventRequest<TestCommand> stop_request{ "", TestCommand::STOP };

	EXPECT_EQ(static_cast<int>(TestState::STOPPED), static_cast<int>(task.get_state()));

	task.rpc_event(start_request);
	EXPECT_EQ(static_cast<int>(TestState::IDLE), static_cast<int>(task.get_state()));

	task.rpc_event(start_request);
	EXPECT_EQ(static_cast<int>(TestState::RUNNING), static_cast<int>(task.get_state()));

	task.rpc_event(stop_request);
	EXPECT_EQ(static_cast<int>(TestState::IDLE), static_cast<int>(task.get_state()));

	task.rpc_event(stop_request);
	EXPECT_EQ(static_cast<int>(TestState::STOPPED), static_cast<int>(task.get_state()));
}

TEST_F(StatementTaskTest, NoStateTransition) {
	StatementTask<TestState, TestCommand> task("test", table, TestState::STOPPED);
	EventRequest<TestCommand> stop_request{ "", TestCommand::STOP };

	EXPECT_EQ(static_cast<int>(TestState::STOPPED), static_cast<int>(task.get_state()));

	task.send_event(stop_request);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_EQ(static_cast<int>(TestState::STOPPED), static_cast<int>(task.get_state()));
}

TEST_F(StatementTaskTest, StateChangedHandler) {
	StatementTask<TestState, TestCommand> task("test", table, TestState::STOPPED);
	EventRequest<TestCommand> start_request{ "", TestCommand::START };
	EventRequest<TestCommand> stop_request{ "", TestCommand::STOP };
	int last_new_state = -1;
	int last_old_state = -1;

	task.state_changed = [&](TestState old_state, TestState new_state) {
		last_old_state = static_cast<int>(old_state);
		last_new_state = static_cast<int>(new_state);
	};

	task.send_event(stop_request);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_EQ(-1, last_old_state);
	EXPECT_EQ(-1, last_new_state);

	task.send_event(start_request);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_EQ(static_cast<int>(TestState::STOPPED), last_old_state);
	EXPECT_EQ(static_cast<int>(TestState::IDLE), last_new_state);
}

TEST_F(StatementTaskTest, SetState) {
	StatementTask<TestState, TestCommand> task("test", table, TestState::STOPPED);
	EventRequest<TestCommand> start_request{ "", TestCommand::START };

	task.set_state(TestState::IDLE);
	EXPECT_EQ(static_cast<int>(TestState::IDLE), static_cast<int>(task.get_state()));

	task.send_event(start_request);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_EQ(static_cast<int>(TestState::RUNNING), static_cast<int>(task.get_state()));
}

TEST_F(StatementTaskTest, BuilderAPI) {
	StatementTask<TestState, TestCommand> task("test", TestState::STOPPED);
	task.on(TestState::STOPPED, TestCommand::START, event_handler, TestState::IDLE)
		.on(TestState::STOPPED, TestCommand::STOP, event_handler, TestState::STOPPED)
		.on(TestState::IDLE, TestCommand::START, event_handler, TestState::RUNNING)
		.on(TestState::IDLE, TestCommand::STOP, event_handler, TestState::STOPPED)
		.on(TestState::RUNNING, TestCommand::START, event_handler, TestState::RUNNING)
		.on(TestState::RUNNING, TestCommand::STOP, event_handler, TestState::IDLE);
	task.start();

	EventRequest<TestCommand> start_request{ "", TestCommand::START };
	EventRequest<TestCommand> stop_request{ "", TestCommand::STOP };

	EXPECT_EQ(static_cast<int>(TestState::STOPPED), static_cast<int>(task.get_state()));

	task.rpc_event(start_request);
	EXPECT_EQ(static_cast<int>(TestState::IDLE), static_cast<int>(task.get_state()));

	task.rpc_event(start_request);
	EXPECT_EQ(static_cast<int>(TestState::RUNNING), static_cast<int>(task.get_state()));

	task.rpc_event(stop_request);
	EXPECT_EQ(static_cast<int>(TestState::IDLE), static_cast<int>(task.get_state()));
}
