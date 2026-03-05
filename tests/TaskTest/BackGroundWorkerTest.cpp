#include "BackGroundWorkerTest.hpp"

#include <thread>

#include "Exception/Exception.hpp"
#include "Task/BackGroundWorker.hpp"

using namespace Framework::Task;

TEST_F(BackGroundWorkerTest, ConstructorAndDestructor) {
	BackGroundWorker worker{ "test",
							 [](BackGroundWorker &, BackGroundWorker::DoTaskEventArgs &) {} };
}

TEST_F(BackGroundWorkerTest, SimpleTask) {
	BackGroundWorker worker{ "test", [&](BackGroundWorker &, BackGroundWorker::DoTaskEventArgs &e) {
								std::this_thread::sleep_for(std::chrono::milliseconds(100));
								e.set_result(true);
							} };
	bool task_result = false;
	bool completed = false;
	worker.task_completed = [&](BackGroundWorker &, BackGroundWorker::TaskCompletedEventArgs &e) {
		task_result = e.result<bool>();
		completed = true;
	};

	worker.run_task_async();

	ASSERT_TRUE(worker.is_busy());

	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	ASSERT_TRUE(completed);
	EXPECT_TRUE(task_result);
}

TEST_F(BackGroundWorkerTest, ReportingProgress) {
	BackGroundWorker worker{ "test",
							 [](BackGroundWorker &worker, BackGroundWorker::DoTaskEventArgs &) {
								 worker.reports_progress(50);
							 } };
	BackGroundWorker::Progress progress = 0;
	worker.progress_changed = [&](BackGroundWorker &,
								  BackGroundWorker::ProgressChangedEventArgs &e) {
		progress = e.progress_percent();
	};

	worker.run_task_async();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(50, progress);
}

TEST_F(BackGroundWorkerTest, Cancellation) {
	BackGroundWorker worker{ "test",
							 [](BackGroundWorker &worker, BackGroundWorker::DoTaskEventArgs &e) {
								 worker.reports_progress(10);
								 std::this_thread::sleep_for(std::chrono::milliseconds(1000));
								 if (worker.cancellation_pending()) {
									 e.set_cancel(true);
									 return;
								 }
								 worker.reports_progress(100);
							 } };
	BackGroundWorker::Progress latest_progress = 0;
	worker.progress_changed = [&](BackGroundWorker &,
								  BackGroundWorker::ProgressChangedEventArgs &e) {
		latest_progress = e.progress_percent();
	};
	bool cancelled = false;
	worker.task_completed = [&](BackGroundWorker &, BackGroundWorker::TaskCompletedEventArgs &e) {
		cancelled = e.cancelled();
	};

	worker.run_task_async();
	worker.cancel_async();

	std::this_thread::sleep_for(std::chrono::milliseconds(1500));

	EXPECT_TRUE(cancelled);
	EXPECT_EQ(10, latest_progress);
}

TEST_F(BackGroundWorkerTest, Exception) {
	BackGroundWorker worker{ "test", [](BackGroundWorker &, BackGroundWorker::DoTaskEventArgs &) {
								throw Framework::Exception(
									"test", Framework::Error::Code::InvalidOperation);
							} };
	Framework::Error::Code error_code = Framework::Error::Code::Unknown;
	worker.task_completed = [&](BackGroundWorker &, BackGroundWorker::TaskCompletedEventArgs &e) {
		error_code = e.error_code();
	};

	worker.run_task_async();

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(static_cast<int>(Framework::Error::Code::InvalidOperation),
			  static_cast<int>(error_code));
}
