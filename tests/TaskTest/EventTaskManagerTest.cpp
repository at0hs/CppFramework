#include "EventTaskManagerTest.hpp"

#include <algorithm>
#include <atomic>
#include <thread>
#include <tuple>

#include "Exception/Error.hpp"
#include "Exception/Exception.hpp"
#include "Task/EventTaskManager.hpp"
#include "Task/MessageTask.hpp"

using namespace Framework::Task;

// テスト用コマンド型
enum class TestCmd : uint8_t {
	Cmd1 = 0,
	Cmd2 = 1,
	SlowCmd = 2,
};

namespace {
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	EventRequest<TestCmd> g_last_request{ "", static_cast<TestCmd>(-1) };

	bool capture_handler(const EventRequest<TestCmd> &req) {
		g_last_request = req;
		return true;
	}

	bool slow_handler(const EventRequest<TestCmd> &req) {
		g_last_request = req;
		std::this_thread::sleep_for(std::chrono::milliseconds(600));
		return true;
	}

	bool false_handler(const EventRequest<TestCmd> & /*req*/) {
		return false;
	}

	const MessageTask<TestCmd>::EventMap kEvents{
		{ TestCmd::Cmd1, { capture_handler } },
		{ TestCmd::Cmd2, { capture_handler } },
		{ TestCmd::SlowCmd, { slow_handler } },
	};

	const MessageTask<TestCmd>::EventMap kFalseEvents{
		{ TestCmd::Cmd1, { false_handler } },
	};
} // namespace

void EventTaskManagerTest::SetUp() {
	g_last_request = EventRequest<TestCmd>{ "", static_cast<TestCmd>(-1) };
}

// --- 登録・クエリ系 ---

TEST_F(EventTaskManagerTest, RegisterAndHasTask) {
	EventTaskManager<TestCmd> manager;
	auto task = std::make_shared<MessageTask<TestCmd>>("task1", kEvents);

	EXPECT_FALSE(manager.has_task("task1"));
	manager.register_task("task1", task);
	EXPECT_TRUE(manager.has_task("task1"));
}

TEST_F(EventTaskManagerTest, UnregisterTask) {
	EventTaskManager<TestCmd> manager;
	auto task = std::make_shared<MessageTask<TestCmd>>("task1", kEvents);
	manager.register_task("task1", task);

	manager.unregister_task("task1");
	EXPECT_FALSE(manager.has_task("task1"));
}

TEST_F(EventTaskManagerTest, UnregisterNonExistentTaskIsNoOp) {
	EventTaskManager<TestCmd> manager;
	// 未登録の名前を unregister しても例外が出ない
	EXPECT_NO_THROW(manager.unregister_task("nonexistent"));
}

TEST_F(EventTaskManagerTest, TaskNames) {
	EventTaskManager<TestCmd> manager;
	auto task1 = std::make_shared<MessageTask<TestCmd>>("t1", kEvents);
	auto task2 = std::make_shared<MessageTask<TestCmd>>("t2", kEvents);
	manager.register_task("alpha", task1);
	manager.register_task("beta", task2);

	auto names = manager.task_names();
	ASSERT_EQ(2U, names.size());
	EXPECT_TRUE(std::ranges::find(names, "alpha") != names.end());
	EXPECT_TRUE(std::ranges::find(names, "beta") != names.end());
}

TEST_F(EventTaskManagerTest, GetTask) {
	EventTaskManager<TestCmd> manager;
	auto task = std::make_shared<MessageTask<TestCmd>>("task1", kEvents);
	manager.register_task("task1", task);

	auto retrieved = manager.get_task("task1");
	EXPECT_EQ(task.get(), retrieved.get());
}

TEST_F(EventTaskManagerTest, GetTaskNotFound) {
	EventTaskManager<TestCmd> manager;

	EXPECT_THROW(std::ignore = manager.get_task("nonexistent"), Framework::Exception);
	try {
		std::ignore = manager.get_task("nonexistent");
	} catch (const Framework::Exception &e) {
		EXPECT_EQ(Framework::Error::Code::InvalidArgument, e.get_code());
	}
}

TEST_F(EventTaskManagerTest, RegisterNullptrThrows) {
	EventTaskManager<TestCmd> manager;

	EXPECT_THROW(manager.register_task("task1", nullptr), Framework::Exception);
	try {
		manager.register_task("task1", nullptr);
	} catch (const Framework::Exception &e) {
		EXPECT_EQ(Framework::Error::Code::InvalidArgument, e.get_code());
	}
}

// --- ルーティング: send_event ---

TEST_F(EventTaskManagerTest, SendEventRouting) {
	EventTaskManager<TestCmd> manager;
	auto task = std::make_shared<MessageTask<TestCmd>>("task1", kEvents);
	task->start();
	manager.register_task("task1", task);

	manager.send_event("task1", { "sender", TestCmd::Cmd1 });
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(TestCmd::Cmd1, g_last_request.get_command());
	EXPECT_EQ("sender", g_last_request.get_from());

	task->stop();
}

TEST_F(EventTaskManagerTest, SendEventConstRefRouting) {
	EventTaskManager<TestCmd> manager;
	auto task = std::make_shared<MessageTask<TestCmd>>("task1", kEvents);
	task->start();
	manager.register_task("task1", task);

	const EventRequest<TestCmd> req{ "sender", TestCmd::Cmd2 };
	manager.send_event("task1", req);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(TestCmd::Cmd2, g_last_request.get_command());

	task->stop();
}

TEST_F(EventTaskManagerTest, SendEventToUnregistered) {
	EventTaskManager<TestCmd> manager;

	EXPECT_THROW(manager.send_event("nonexistent", { "sender", TestCmd::Cmd1 }),
				 Framework::Exception);
}

// --- ルーティング: rpc_event ---

TEST_F(EventTaskManagerTest, RpcEventRouting) {
	EventTaskManager<TestCmd> manager;
	auto task = std::make_shared<MessageTask<TestCmd>>("task1", kEvents);
	task->start();
	manager.register_task("task1", task);

	const bool result = manager.rpc_event("task1", { "sender", TestCmd::Cmd1 });
	EXPECT_TRUE(result);
	EXPECT_EQ(TestCmd::Cmd1, g_last_request.get_command());

	task->stop();
}

TEST_F(EventTaskManagerTest, RpcEventReturnsFalse) {
	EventTaskManager<TestCmd> manager;
	auto task = std::make_shared<MessageTask<TestCmd>>("task1", kFalseEvents);
	task->start();
	manager.register_task("task1", task);

	EXPECT_FALSE(manager.rpc_event("task1", { "sender", TestCmd::Cmd1 }));

	task->stop();
}

TEST_F(EventTaskManagerTest, RpcEventTimeout) {
	EventTaskManager<TestCmd> manager;
	auto task = std::make_shared<MessageTask<TestCmd>>("task1", kEvents);
	task->start();
	manager.register_task("task1", task);

	// SlowCmd は 600ms かかるので 200ms でタイムアウト
	const bool result =
		manager.rpc_event("task1", { "sender", TestCmd::SlowCmd }, std::chrono::milliseconds(200));
	EXPECT_FALSE(result);

	task->stop();
}

TEST_F(EventTaskManagerTest, RpcEventToUnregistered) {
	EventTaskManager<TestCmd> manager;

	EXPECT_THROW(manager.rpc_event("nonexistent", { "sender", TestCmd::Cmd1 }),
				 Framework::Exception);
}

// --- 複数タスクの独立ルーティング ---

TEST_F(EventTaskManagerTest, MultipleTasksIndependentRouting) {
	EventTaskManager<TestCmd> manager;

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	static std::atomic<int> handler1_count{ 0 };
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	static std::atomic<int> handler2_count{ 0 };

	handler1_count = 0;
	handler2_count = 0;

	const MessageTask<TestCmd>::EventMap events1{
		{ TestCmd::Cmd1, { [](const EventRequest<TestCmd> &) {
			  ++handler1_count;
			  return true;
		  } } },
	};
	const MessageTask<TestCmd>::EventMap events2{
		{ TestCmd::Cmd1, { [](const EventRequest<TestCmd> &) {
			  ++handler2_count;
			  return true;
		  } } },
	};

	auto task1 = std::make_shared<MessageTask<TestCmd>>("t1", events1);
	auto task2 = std::make_shared<MessageTask<TestCmd>>("t2", events2);
	task1->start();
	task2->start();
	manager.register_task("task1", task1);
	manager.register_task("task2", task2);

	manager.send_event("task1", { "sender", TestCmd::Cmd1 });
	manager.send_event("task1", { "sender", TestCmd::Cmd1 });
	manager.send_event("task2", { "sender", TestCmd::Cmd1 });
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(2, handler1_count.load());
	EXPECT_EQ(1, handler2_count.load());

	task1->stop();
	task2->stop();
}
