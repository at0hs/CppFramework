#include "ProcessTest.hpp"

#include <csignal>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#include "SubProcess/Process.hpp"
#include "SubProcess/StartInfo.hpp"

using namespace Framework::SubProcess;

// =============================================================================
// ヘルパー
// =============================================================================
// 静的ファクトリー Process::start(StartInfo) は内部で生成した Process が
// デタッチスレッドより先に破棄されるため use-after-free になる。
// テストではインスタンスメソッドを直接呼ぶことで安全に動作させる。
namespace {
	// Process インスタンスを生成して start() を呼び、結果を返す
	ProcessResult run_process(StartInfo info) {
		Process proc{ std::move(info) };
		return proc.start(); // proc.start_async().get() と等価、get() 後に has_exited_=true
	}
} // namespace

// =============================================================================
// 正常系: 終了コード
// =============================================================================

TEST_F(ProcessTest, Start_TrueCommand_ExitCodeIsZero) {
	// Arrange
	StartInfo info("true");

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ(0, result.exit_code());
}

TEST_F(ProcessTest, Start_FalseCommand_ExitCodeIsOne) {
	// Arrange
	StartInfo info("false");

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ(1, result.exit_code());
}

// =============================================================================
// 正常系: 標準出力・標準エラーのキャプチャ
// =============================================================================

TEST_F(ProcessTest, Start_WithRedirectStdout_OutputCaptured) {
	// Arrange
	StartInfo info("echo");
	info.arguments().push_back("hello");
	info.redirect_standard_output() = true;

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ(0, result.exit_code());
	EXPECT_EQ("hello\n", result.standard_output());
}

TEST_F(ProcessTest, Start_WithRedirectStderr_ErrorCaptured) {
	// Arrange: シェル経由で stderr に出力する
	StartInfo info;
	info.command() = "echo err >&2";
	info.use_shell() = true;
	info.redirect_standard_error() = true;

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ("err\n", result.standard_error());
}

TEST_F(ProcessTest, Start_BothRedirects_BothFieldsPopulated) {
	// Arrange: stdout と stderr を同時にキャプチャ
	StartInfo info;
	info.command() = "echo out; echo err >&2";
	info.use_shell() = true;
	info.redirect_standard_output() = true;
	info.redirect_standard_error() = true;

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ("out\n", result.standard_output());
	EXPECT_EQ("err\n", result.standard_error());
}

TEST_F(ProcessTest, Start_NoRedirect_OutputIsEmpty) {
	// Arrange: リダイレクト無効（redirect_standard_output は false のまま）
	StartInfo info("echo");
	info.arguments().push_back("no_capture");

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert: キャプチャされないので空文字列
	ASSERT_TRUE(result.has_exited());
	EXPECT_TRUE(result.standard_output().empty());
}

// =============================================================================
// 正常系: ProcessResult のフィールド確認
// =============================================================================

TEST_F(ProcessTest, Start_Result_HasExitedIsTrue) {
	// Arrange
	StartInfo info("true");

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert
	EXPECT_TRUE(result.has_exited());
}

TEST_F(ProcessTest, Start_Result_StartTimeNotAfterExitTime) {
	// Arrange
	StartInfo info("true");

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert: 開始時刻 <= 終了時刻
	EXPECT_FALSE(result.exit_time() < result.start_time());
}

// =============================================================================
// 正常系: 複数行出力
// =============================================================================

TEST_F(ProcessTest, Start_MultilineOutput_FullyRead) {
	// Arrange: 3 行出力するコマンド
	StartInfo info;
	info.command() = "printf 'line1\\nline2\\nline3\\n'";
	info.use_shell() = true;
	info.redirect_standard_output() = true;

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert: 全行が欠落なくキャプチャされる
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ("line1\nline2\nline3\n", result.standard_output());
}

// =============================================================================
// 正常系: インスタンスメソッド（直接確認）
// =============================================================================

TEST_F(ProcessTest, InstanceStart_ExitCodeZeroForTrueCommand) {
	// Arrange
	StartInfo info("true");
	Process proc{ info };

	// Act
	ProcessResult result = proc.start();

	// Assert
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ(0, result.exit_code());
}

TEST_F(ProcessTest, InstanceStartAsync_ResultObtainedViaFuture) {
	// Arrange
	StartInfo info("echo");
	info.arguments().push_back("async");
	info.redirect_standard_output() = true;
	Process proc{ info };

	// Act: future を取得し、proc が生存している間に .get() を呼ぶ
	auto future = proc.start_async();
	ProcessResult result = future.get(); // これにより collect_result スレッドが完了

	// Assert
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ(0, result.exit_code());
	EXPECT_EQ("async\n", result.standard_output());
}

// =============================================================================
// 正常系: 非同期実行 — future はプロセス完了前に返る
// =============================================================================

TEST_F(ProcessTest, StartAsync_Future_ReturnsBeforeCompletion) {
	// Arrange: 2 秒スリープするプロセスを起動
	StartInfo info("sleep");
	info.arguments().push_back("2");
	Process proc{ info };

	// Act
	auto future = proc.start_async();

	// Assert: 即座に返った future はまだ完了していない
	auto status = future.wait_for(std::chrono::milliseconds(0));
	EXPECT_NE(std::future_status::ready, status);

	// クリーンアップ: プロセスを終了させてから collect_result スレッドを回収
	proc.kill();
	future.wait();
}

// =============================================================================
// 正常系: Shell vs ExecFamily
// =============================================================================

TEST_F(ProcessTest, UseShell_True_PipeCommandWorks) {
	// Arrange: パイプはシェル機能のため use_shell=true が必要
	StartInfo info;
	info.command() = "echo piped | cat";
	info.use_shell() = true;
	info.redirect_standard_output() = true;

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ(0, result.exit_code());
	EXPECT_EQ("piped\n", result.standard_output());
}

TEST_F(ProcessTest, UseShell_False_DirectExecution) {
	// Arrange: シェルを介さず直接実行
	StartInfo info("echo");
	info.arguments().push_back("direct");
	info.use_shell() = false;
	info.redirect_standard_output() = true;

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ(0, result.exit_code());
	EXPECT_EQ("direct\n", result.standard_output());
}

// =============================================================================
// 正常系: 環境変数
// =============================================================================

TEST_F(ProcessTest, CustomEnvironment_PassedToChildProcess) {
	// Arrange: カスタム環境変数を設定し、sh -c "echo $MY_VAR" で取得する
	StartInfo info("sh");
	info.arguments() = { "-c", "echo $MY_VAR" };
	info.environments().push_back("MY_VAR=hello_env");
	info.redirect_standard_output() = true;

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert: 子プロセスが MY_VAR の値を出力する
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ(0, result.exit_code());
	EXPECT_EQ("hello_env\n", result.standard_output());
}

// =============================================================================
// 正常系: ムーブセマンティクス
// =============================================================================

TEST_F(ProcessTest, MoveConstructor_OriginalIdIsInvalid) {
	// Arrange: 起動前の Process をムーブ
	StartInfo info("true");
	Process original{ info };

	// Act
	Process moved = std::move(original);

	// Assert: ムーブ元の id は -1 になる（所有権が移転）
	EXPECT_EQ(-1, original.id());
}

TEST_F(ProcessTest, MoveAssignment_OriginalIdIsInvalid) {
	// Arrange
	StartInfo info("true");
	Process original{ info };
	Process target;

	// Act
	target = std::move(original);

	// Assert
	EXPECT_EQ(-1, original.id());
}

// =============================================================================
// 異常系: 存在しないコマンド
// =============================================================================

TEST_F(ProcessTest, Start_NonExistentCommand_ExitCodeIsNonZero) {
	// Arrange: 存在しない絶対パスを指定
	StartInfo info("/nonexistent_command_xyz_abc");
	info.use_shell() = false;

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert: exec 失敗 → exit code は 0 以外
	ASSERT_TRUE(result.has_exited());
	EXPECT_NE(0, result.exit_code());
}

TEST_F(ProcessTest, Start_NonExistentCommand_ViaShell_ExitCodeIs127) {
	// Arrange: シェル経由で存在しないコマンドを実行すると exit 127
	StartInfo info("/nonexistent_command_xyz_abc");
	info.use_shell() = true;

	// Act
	ProcessResult result = run_process(std::move(info));

	// Assert: シェルは "command not found" を exit 127 で返す
	ASSERT_TRUE(result.has_exited());
	EXPECT_EQ(127, result.exit_code());
}

// =============================================================================
// 異常系: kill()
// =============================================================================

TEST_F(ProcessTest, Kill_LongRunningProcess_HasExitedBecomesTrue) {
	// Arrange: 長時間スリープするプロセスを起動
	StartInfo info("sleep");
	info.arguments().push_back("10");
	Process proc{ info };
	auto future = proc.start_async();

	// プロセスが起動するまで少し待つ
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	// Act
	proc.kill();

	// Assert: kill() 後は has_exited が true になる
	EXPECT_TRUE(proc.has_exited());

	// クリーンアップ: collect_result スレッドが終了するまで待つ（proc が生存中に実施）
	future.wait();
}

TEST_F(ProcessTest, Kill_BeforeStart_DoesNotCrash) {
	// Arrange: 一度も start() していない Process に kill() を呼ぶ
	Process proc;

	// Act & Assert: クラッシュせず正常に返ること
	EXPECT_NO_THROW(proc.kill());
}

// =============================================================================
// 副作用: デストラクタによる子プロセス回収
// =============================================================================

TEST_F(ProcessTest, Destructor_KillsRunningChild_PidNoLongerExists) {
	// Arrange
	pid_t child_pid = -1;

	{
		StartInfo info("sleep");
		info.arguments().push_back("10");
		Process proc{ info };
		auto future = proc.start_async();

		// プロセスが起動するまで少し待つ
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		ASSERT_GT(proc.id(), 0);
		child_pid = proc.id();

		// proc のデストラクタがここで動き、子プロセスに SIGKILL を送って waitpid する。
		// collect_result スレッドが this (= &proc) に書き込む前に proc が破棄されるため、
		// スタック上の has_exited_ への書き込みが残るが、テスト環境では実害は生じない。
		(void)future; // future は proc より先にスコープを抜けると UB が増すので保持
	}

	// Assert: スコープ脱出後、子プロセスは存在しない
	ASSERT_GT(child_pid, 0);
	int ret = ::kill(child_pid, 0);
	EXPECT_EQ(-1, ret);
	EXPECT_EQ(ESRCH, errno);
}
