#include "ProcessSpawnerTest.hpp"

#include <sys/wait.h>

#include "SubProcess/ProcessSpawner.hpp"

using namespace Framework::SubProcess;

namespace {

// テスト用ヘルパー: spawn した子プロセスを回収して終了コードを返す
int wait_child(pid_t pid) {
	int status{ 0 };
	::waitpid(pid, &status, 0);
	return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

// テスト用関数: 整数ポインタが指す値をそのまま返す
int return_value(const int* arg) {
	return *arg;
}

// テスト用関数: nullptr を受け取っても安全に動作する
int handle_null_safely(const int* arg) {
	if (arg == nullptr) {
		return 0;
	}
	return *arg;
}

// テスト用: 構造体引数
struct SpawnArgs {
	int value;
};

int return_struct_value(SpawnArgs* arg) {
	return arg->value;
}

} // namespace

// =============================================================================
// 正常系: spawn() の基本動作
// =============================================================================

TEST_F(ProcessSpawnerTest, Spawn_SimpleFunction_ReturnsPositivePid) {
	// Arrange
	int exit_value = 0;

	// Act
	pid_t pid = ProcessSpawner::spawn(return_value, &exit_value);

	// Assert: 正の PID が返る（> 0 は親プロセス側の戻り値）
	ASSERT_GT(pid, 0);

	// 子プロセスを回収してゾンビ防止
	wait_child(pid);
}

TEST_F(ProcessSpawnerTest, Spawn_ReturnValue_PropagatedViaExitCode) {
	// Arrange: 子プロセスが 42 を返す
	int exit_value = 42;

	// Act
	pid_t pid = ProcessSpawner::spawn(return_value, &exit_value);
	ASSERT_GT(pid, 0);
	int actual_exit = wait_child(pid);

	// Assert: waitpid で取得した終了コードが 42 と一致する
	EXPECT_EQ(42, actual_exit);
}

TEST_F(ProcessSpawnerTest, Spawn_StructPointerArgument_WorksCorrectly) {
	// Arrange: 構造体ポインタをテンプレート型引数として渡す
	SpawnArgs args{ 7 };

	// Act
	pid_t pid = ProcessSpawner::spawn(return_struct_value, &args);
	ASSERT_GT(pid, 0);
	int actual_exit = wait_child(pid);

	// Assert
	EXPECT_EQ(7, actual_exit);
}

// =============================================================================
// 境界値: nullptr 引数
// =============================================================================

TEST_F(ProcessSpawnerTest, Spawn_NullArgument_FunctionExecutesWithoutCrash) {
	// Arrange: nullptr を渡しても安全に処理する関数を使用
	// Act
	pid_t pid = ProcessSpawner::spawn(handle_null_safely, (int *)nullptr);
	ASSERT_GT(pid, 0);
	int actual_exit = wait_child(pid);

	// Assert: クラッシュせず正常終了（exit code = 0）
	EXPECT_EQ(0, actual_exit);
}
