#include "Timer.hpp"
#include "gtest/gtest.h"

TEST(TimerTest, Oneshot) {
	Timer timer{ std::chrono::milliseconds(500) };
	int counter = 0;

	timer.add_listener([&] { counter++; });

	timer.start();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	timer.stop();

	EXPECT_EQ(1, counter);
}

TEST(TimerTest, Cycle) {
	Timer timer{ std::chrono::milliseconds(500) };
	int counter = 0;

	timer.auto_restart(true);

	timer.add_listener([&] { counter++; });

	timer.start();
	std::this_thread::sleep_for(std::chrono::seconds(2));
	timer.stop();

	EXPECT_GE(counter, 3);
}

TEST(TimerTest, Listeners) {
	Timer timer{ std::chrono::milliseconds(500) };
	int counter{ 0 };

	timer.add_listener([&] { counter++; });
	timer.add_listener([&] { counter++; });

	timer.start();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	timer.stop();

	EXPECT_EQ(2, counter);
}

TEST(TimerTest, StartStop) {
	Timer timer{ std::chrono::milliseconds(500) };
	int counter{ 0 };

	timer.add_listener([&] { counter++; });

	timer.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(700));
	timer.stop();

	timer.start();
	std::this_thread::sleep_for(std::chrono::milliseconds(700));
	timer.stop();

	EXPECT_EQ(2, counter);
}
