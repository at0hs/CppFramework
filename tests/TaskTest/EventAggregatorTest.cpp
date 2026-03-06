#include "EventAggregatorTest.hpp"

#include "Task/MessageEventAggregator.hpp"

using namespace Framework::Task;

namespace {
	constexpr EventRequest<>::Command kTestCommand1 = 0;
	constexpr EventRequest<>::Command kTestCommand2 = 1;
} // namespace

TEST_F(EventAggregatorTest, InitializeWithEventMap) {
	std::string name;
	EventRequest<>::Command command = -1;
	std::any payload;
	const MessageEventAggregator<>::EventMap events{
		{ kTestCommand1, MessageEventAggregator<>::MessageInfo([&](const EventRequest<> &req) {
			  name = req.get_from();
			  command = req.get_command();
			  payload = req.get_payload();
			  return true;
		  }) },
		{ kTestCommand2, MessageEventAggregator<>::MessageInfo([&](const EventRequest<> &req) {
			  name = req.get_from();
			  command = req.get_command();
			  payload = req.get_payload();
			  return false;
		  }) }
	};
	EventRequest<> command1_request("EventAggregatorTest", kTestCommand1, std::string("Test"));
	EventRequest<> command2_request("EventAggregatorTest", kTestCommand2, std::string("Test"));
	MessageEventAggregator<> aggregator(events);

	EXPECT_TRUE(aggregator.publish(kTestCommand1, command1_request));
	EXPECT_STREQ("EventAggregatorTest", name.c_str());
	EXPECT_EQ(kTestCommand1, command);
	EXPECT_STREQ("Test", std::any_cast<std::string>(payload).c_str());

	EXPECT_FALSE(aggregator.publish(kTestCommand2, command2_request));
	EXPECT_STREQ("EventAggregatorTest", name.c_str());
	EXPECT_EQ(kTestCommand2, command);
	EXPECT_STREQ("Test", std::any_cast<std::string>(payload).c_str());
}

TEST_F(EventAggregatorTest, InitializeWithInitializerList) {
	std::string name;
	EventRequest<>::Command command = -1;
	std::any payload;
	const std::initializer_list<MessageEventAggregator<>::MessageEvent> events{
		{ kTestCommand1, MessageEventAggregator<>::MessageInfo([&](const EventRequest<> &req) {
			  name = req.get_from();
			  command = req.get_command();
			  payload = req.get_payload();
			  return true;
		  }) },
		{ kTestCommand2, MessageEventAggregator<>::MessageInfo([&](const EventRequest<> &req) {
			  name = req.get_from();
			  command = req.get_command();
			  payload = req.get_payload();
			  return false;
		  }) }
	};
	EventRequest<> command1_request("EventAggregatorTest", kTestCommand1, std::string("Test"));
	EventRequest<> command2_request("EventAggregatorTest", kTestCommand2, std::string("Test"));
	MessageEventAggregator<> aggregator(events);

	EXPECT_TRUE(aggregator.publish(kTestCommand1, command1_request));
	EXPECT_STREQ("EventAggregatorTest", name.c_str());
	EXPECT_EQ(kTestCommand1, command);
	EXPECT_STREQ("Test", std::any_cast<std::string>(payload).c_str());

	EXPECT_FALSE(aggregator.publish(kTestCommand2, command2_request));
	EXPECT_STREQ("EventAggregatorTest", name.c_str());
	EXPECT_EQ(kTestCommand2, command);
	EXPECT_STREQ("Test", std::any_cast<std::string>(payload).c_str());
}

TEST_F(EventAggregatorTest, AddMethod) {
	EventRequest<>::Command received = -1;
	MessageEventAggregator<> aggregator;
	aggregator.add(0, [&](const EventRequest<> &req) {
		received = req.get_command();
		return true;
	});

	EventRequest<> req("test", 0);
	EXPECT_TRUE(aggregator.publish(0, req));
	EXPECT_EQ(0, received);
}
