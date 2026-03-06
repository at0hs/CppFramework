#include "TaskPoolTest.hpp"

#include <atomic>
#include <thread>

#include "Task/TaskPool.hpp"

using namespace Framework::Task;

namespace {
	void set_affinity(int n) {
		cpu_set_t cpu_set;
		CPU_ZERO(&cpu_set);
		for (int i = 0; i < n; i++) {
			CPU_SET(i, &cpu_set);
		}
		sched_setaffinity(0, sizeof(cpu_set), &cpu_set);
	}
} // namespace

TEST_F(TaskPoolTest, Constructor) {
	int num_cpu = 5;
	set_affinity(num_cpu);

	TaskPool pool1{ "Test" };
	EXPECT_EQ(num_cpu, pool1.concurrency());

	TaskPool pool2{ "Test", 2 };
	EXPECT_EQ(2, pool2.concurrency());
}

TEST_F(TaskPoolTest, Enqueue) {
	TaskPool pool{ "Test" };
	std::atomic_int counter = 0;

	for (int i = 0; i < 10; i++) {
		pool.enqueue([&counter] { counter++; });
	}
	pool.stop();

	EXPECT_EQ(10, counter);
}

TEST_F(TaskPoolTest, CountRunningTasks) {
	TaskPool pool{ "Test", 1 };
	bool paused = true;
	int num_tasks = 10;

	for (int i = 0; i < num_tasks; i++) {
		pool.enqueue([&paused] {
			while (paused) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		});
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(1, pool.count_running_tasks());
	paused = false;
}

TEST_F(TaskPoolTest, CountWaitingTasks) {
	TaskPool pool{ "Test", 1 };

	EXPECT_EQ(0, pool.count_waiting_tasks());

	pool.enqueue([] { std::this_thread::sleep_for(std::chrono::milliseconds(1000)); });
	pool.enqueue([] { std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(1, pool.count_waiting_tasks());

	pool.stop();

	EXPECT_EQ(0, pool.count_waiting_tasks());
}

TEST_F(TaskPoolTest, IsEmpty) {
	TaskPool pool{ "Test" };

	EXPECT_TRUE(pool.is_empty());

	pool.enqueue([] { std::this_thread::sleep_for(std::chrono::milliseconds(1000)); });
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_TRUE(pool.is_empty());

	pool.stop();

	EXPECT_TRUE(pool.is_empty());
}

TEST_F(TaskPoolTest, ClearWaitingTasks) {
	TaskPool pool{ "Test", 1 };
	std::atomic_bool paused = true;

	// 並行数1で10タスクをenqueue。先頭1つが実行中、残り9つが待機中になる
	for (int i = 0; i < 10; i++) {
		pool.enqueue([&paused] {
			while (paused) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		});
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_EQ(9, pool.count_waiting_tasks());

	pool.clear_waiting_tasks();
	EXPECT_EQ(0, pool.count_waiting_tasks());

	// 実行中タスクを完了させてから終了
	paused = false;
	pool.stop();
}
