#include "MessageTaskTest.hpp"

#include <thread>

#include "Exception/Error.hpp"
#include "Exception/Exception.hpp"
#include "Task/MessageTask.hpp"

using namespace Framework::Task;


enum class Commands : uint8_t {
	TestCommand1 = 0,
	TestCommand2 = 1,
};

namespace {
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	 EventRequest<Commands> last_request{ "", static_cast<Commands>(-1) };
	
	 bool test_handler(const EventRequest<Commands> &req) {
		last_request = req;
		return true;
	}
	
	 bool rpc_test_handler(const EventRequest<Commands> &req) {
		last_request = req;
		std::this_thread::sleep_for(std::chrono::milliseconds(600));
		return req.get_command() == Commands::TestCommand1;
	}
	
	 bool throw_exception(const EventRequest<Commands> &req) {
		if (req.get_command() == Commands::TestCommand1) {
			throw Framework::Exception("Command1Exception", Framework::Error::Code::Success);
		}
		throw Framework::Exception("Command2Exception", Framework::Error::Code::OutOfRange);
	}
}

void MessageTaskTest::SetUp() {
	last_request = EventRequest<Commands>{ "", static_cast<Commands>(-1) };
}

const MessageTask<Commands>::EventMap kEvents{
	{ Commands::TestCommand1, { test_handler } },
	{ Commands::TestCommand2, { test_handler } }
};

const MessageTask<Commands>::EventMap kRpcEvents{
	{ Commands::TestCommand1, { rpc_test_handler } },
	{ Commands::TestCommand2, { rpc_test_handler } }
};

const MessageTask<Commands>::EventMap kExceptionEvents{
	{ Commands::TestCommand1, { throw_exception } },
	{ Commands::TestCommand2, { throw_exception } }
};

TEST_F(MessageTaskTest, StartStop) {
	MessageTask<Commands> task("TestTask", kEvents);
	bool on_start_called = false;
	bool on_finish_called = false;

	task.set_on_start([&on_start_called]() { on_start_called = true; });
	task.set_on_finish([&on_finish_called]() { on_finish_called = true; });

	task.start();
	EXPECT_TRUE(task.is_running());
	task.stop();
	EXPECT_FALSE(task.is_running());

	EXPECT_TRUE(on_start_called);
	EXPECT_TRUE(on_finish_called);
}

TEST_F(MessageTaskTest, SendEventWithEventMapInitialization) {
	std::string command1_payload = "Command1";
	std::string command2_payload = "Command2";
	MessageTask<Commands> task("TestTask", kEvents);

	task.start();

	task.send_event({ "Test", Commands::TestCommand1, command1_payload });
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ("Test", last_request.get_from());
	EXPECT_EQ(Commands::TestCommand1,
			  last_request.get_command());
	EXPECT_STREQ(command1_payload.c_str(),
				 last_request.get_payload_as<std::string>().c_str());

	EventRequest request{ "Test", Commands::TestCommand2, command2_payload };
	task.send_event(request);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ("Test", last_request.get_from());
	EXPECT_EQ(Commands::TestCommand2,
			  last_request.get_command());
	EXPECT_STREQ(command2_payload.c_str(),
				 last_request.get_payload_as<std::string>().c_str());
}

TEST_F(MessageTaskTest, SendEventWithInitializerList) {
	std::string command1_payload = "Command1";
	std::string command2_payload = "Command2";
	MessageTask<Commands> task(
		"TestTask",
		{ { Commands::TestCommand1, { test_handler } },
		  { Commands::TestCommand2,
			{ test_handler } } });

	task.start();

	task.send_event({ "Test", Commands::TestCommand1, command1_payload });
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ("Test", last_request.get_from());
	EXPECT_EQ(Commands::TestCommand1,
			  last_request.get_command());
	EXPECT_STREQ(command1_payload.c_str(),
				 last_request.get_payload_as<std::string>().c_str());

	EventRequest request{ "Test", Commands::TestCommand2, command2_payload };
	task.send_event(request);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ("Test", last_request.get_from());
	EXPECT_EQ(Commands::TestCommand2,
			  last_request.get_command());
	EXPECT_STREQ(command2_payload.c_str(),
				 last_request.get_payload_as<std::string>().c_str());
}

TEST_F(MessageTaskTest, RpcEvent) {
	std::string command1_payload = "Command1";
	std::string command2_payload = "Command2";
	MessageTask<Commands> task("TestTask", kRpcEvents);

	task.start();

	EXPECT_TRUE(task.rpc_event(
		{ "Test", Commands::TestCommand1, command1_payload }));
	EXPECT_EQ("Test", last_request.get_from());
	EXPECT_EQ(Commands::TestCommand1,
			  last_request.get_command());
	EXPECT_STREQ(command1_payload.c_str(),
				 last_request.get_payload_as<std::string>().c_str());

	EventRequest request{ "Test", Commands::TestCommand2, command2_payload };
	EXPECT_FALSE(task.rpc_event(request));
	EXPECT_EQ("Test", last_request.get_from());
	EXPECT_EQ(Commands::TestCommand2,
			  last_request.get_command());
	EXPECT_STREQ(command2_payload.c_str(),
				 last_request.get_payload_as<std::string>().c_str());
}

TEST_F(MessageTaskTest, RpcEventTimeout) {
	std::string command1_payload = "Command1";
	MessageTask<Commands> task("TestTask", kRpcEvents);

	task.start();

	EXPECT_FALSE(task.rpc_event(
		{ "Test", Commands::TestCommand1, command1_payload },
		std::chrono::milliseconds(500)));
}

TEST_F(MessageTaskTest, ExceptionInHandler) {
	std::string command1_payload = "Command1";
	std::string command2_payload = "Command2";
	MessageTask<Commands> task("TestTask", kExceptionEvents);

	task.start();

	Framework::Exception exception{ "", Framework::Error::Code::Unknown };

	try {
		task.rpc_event(
			{ "Test", Commands::TestCommand1, command1_payload });
	} catch (const Framework::Exception &e) {
		exception = e;
	}
	EXPECT_EQ(static_cast<int>(Framework::Error::Code::Success),
			  static_cast<int>(exception.get_code()));

	try {
		EventRequest request{ "Test", Commands::TestCommand2,
							  command2_payload };
		task.rpc_event(request);
	} catch (const Framework::Exception &e) {
		exception = e;
	}
	EXPECT_EQ(static_cast<int>(Framework::Error::Code::OutOfRange),
			  static_cast<int>(exception.get_code()));
}
