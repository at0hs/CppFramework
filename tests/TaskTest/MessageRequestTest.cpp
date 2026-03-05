#include "MessageRequestTest.hpp"

#include "Task/EventRequest.hpp"

using namespace Framework::Task;

TEST_F(EventRequestTest, FromAndCommand) {
	const std::string from = "Test";
	const EventRequest<>::Command command = 0;
	EventRequest request(from, command);

	EXPECT_STREQ(from.c_str(), request.get_from().c_str());
	EXPECT_EQ(command, request.get_command());
}

TEST_F(EventRequestTest, Payload) {
	const std::string input = "Test";
	EventRequest<> request("Test", 0, input);

	ASSERT_TRUE(request.has_payload());

	const std::any &payload = request.get_payload();
	std::string data = std::any_cast<std::string>(payload);
	EXPECT_STREQ(input.c_str(), request.get_payload_as<std::string>().c_str());
	EXPECT_STREQ(input.c_str(), data.c_str());

	EventRequest request2("Test", 0);
	EXPECT_FALSE(request2.has_payload());
}

TEST_F(EventRequestTest, CopyAssignment) {
	const std::string from = "Test";
	const EventRequest<>::Command command = 0;
	const std::string input = "TestPayload";
	EventRequest request(from, command, input);

	EventRequest request2;
	request2 = request;

	EXPECT_STREQ(from.c_str(), request2.get_from().c_str());
	EXPECT_EQ(command, request2.get_command());
	EXPECT_TRUE(request2.has_payload());
	EXPECT_STREQ(input.c_str(), request2.get_payload_as<std::string>().c_str());
}
