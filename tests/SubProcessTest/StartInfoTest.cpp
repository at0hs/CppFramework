#include "StartInfoTest.hpp"

#include "SubProcess/StartInfo.hpp"

using namespace Framework::SubProcess;

// =============================================================================
// 正常系: デフォルトコンストラクタ
// =============================================================================

TEST_F(StartInfoTest, DefaultConstructor_CommandAndContainersAreEmpty) {
	// Arrange & Act
	StartInfo info;

	// Assert
	EXPECT_TRUE(info.command().empty());
	EXPECT_TRUE(info.arguments().empty());
	EXPECT_TRUE(info.environments().empty());
}

TEST_F(StartInfoTest, DefaultConstructor_AllFlagsAreFalse) {
	// Arrange & Act
	StartInfo info;

	// Assert
	EXPECT_FALSE(info.redirect_standard_output());
	EXPECT_FALSE(info.redirect_standard_error());
	EXPECT_FALSE(info.use_shell());
}

// =============================================================================
// 正常系: コマンド指定コンストラクタ
// =============================================================================

TEST_F(StartInfoTest, CommandConstructor_StoresCommandCorrectly) {
	// Arrange & Act
	StartInfo info("echo");

	// Assert
	EXPECT_EQ("echo", info.command());
}

// =============================================================================
// 正常系: 非 const 参照ゲッターによる変更
// =============================================================================

TEST_F(StartInfoTest, NonConstReference_ArgumentsCanBePushedBack) {
	// Arrange
	StartInfo info("echo");

	// Act
	info.arguments().push_back("hello");
	info.arguments().push_back("world");

	// Assert
	ASSERT_EQ(2U, info.arguments().size());
	EXPECT_EQ("hello", info.arguments()[0]);
	EXPECT_EQ("world", info.arguments()[1]);
}

TEST_F(StartInfoTest, NonConstReference_EnvironmentsCanBePushedBack) {
	// Arrange
	StartInfo info;

	// Act
	info.environments().push_back("PATH=/usr/bin");
	info.environments().push_back("HOME=/root");

	// Assert
	ASSERT_EQ(2U, info.environments().size());
	EXPECT_EQ("PATH=/usr/bin", info.environments()[0]);
	EXPECT_EQ("HOME=/root", info.environments()[1]);
}

// =============================================================================
// 正常系: get_command_line()
// =============================================================================

TEST_F(StartInfoTest, GetCommandLine_CommandOnly_ReturnsCommand) {
	// Arrange
	StartInfo info("ls");

	// Act
	auto cmd = info.get_command_line();

	// Assert
	EXPECT_EQ("ls", cmd);
}

TEST_F(StartInfoTest, GetCommandLine_WithArguments_SpaceSeparatedString) {
	// Arrange
	StartInfo info("echo");
	info.arguments().push_back("hello");
	info.arguments().push_back("world");

	// Act
	auto cmd = info.get_command_line();

	// Assert
	EXPECT_EQ("echo hello world", cmd);
}

// =============================================================================
// 正常系: c_style_environments()
// =============================================================================

TEST_F(StartInfoTest, CStyleEnvironments_MultipleEntries_NullTerminated) {
	// Arrange
	StartInfo info;
	info.environments().push_back("A=1");
	info.environments().push_back("B=2");

	// Act
	auto envs = info.c_style_environments();

	// Assert: 要素数 = entries + 1(nullptr 終端)
	ASSERT_EQ(3U, envs.size());
	EXPECT_EQ(nullptr, envs.back());
}

TEST_F(StartInfoTest, CStyleEnvironments_ContentsMatchSource) {
	// Arrange
	StartInfo info;
	info.environments().push_back("KEY=value");
	info.environments().push_back("FOO=bar");

	// Act
	auto envs = info.c_style_environments();

	// Assert: 文字列の内容が一致する
	ASSERT_GE(envs.size(), 2U);
	EXPECT_STREQ("KEY=value", envs[0]);
	EXPECT_STREQ("FOO=bar", envs[1]);
}

// =============================================================================
// 境界値: 引数なし・環境変数なし
// =============================================================================

TEST_F(StartInfoTest, GetCommandLine_EmptyArguments_ReturnsCommandOnly) {
	// Arrange
	StartInfo info("echo");
	// arguments は空のまま

	// Act
	auto cmd = info.get_command_line();

	// Assert
	EXPECT_EQ("echo", cmd);
}

TEST_F(StartInfoTest, CStyleEnvironments_EmptyList_ReturnsNullptrOnly) {
	// Arrange
	StartInfo info;
	// environments は空のまま

	// Act
	auto envs = info.c_style_environments();

	// Assert: 要素数 1（nullptr のみ）
	ASSERT_EQ(1U, envs.size());
	EXPECT_EQ(nullptr, envs[0]);
}

// =============================================================================
// 境界値: 空コマンド
// =============================================================================

TEST_F(StartInfoTest, GetCommandLine_EmptyCommand_DoesNotCrash) {
	// Arrange
	StartInfo info;
	// command は空

	// Act & Assert: クラッシュしないこと
	EXPECT_NO_THROW({ [[maybe_unused]] auto cmd = info.get_command_line(); });
}

// =============================================================================
// 副作用確認: フラグの独立性
// =============================================================================

TEST_F(StartInfoTest, SetRedirectStdout_DoesNotAffectStderrFlag) {
	// Arrange
	StartInfo info;

	// Act
	info.redirect_standard_output() = true;

	// Assert: stdout フラグのみ変化し、stderr フラグは false のまま
	EXPECT_TRUE(info.redirect_standard_output());
	EXPECT_FALSE(info.redirect_standard_error());
}

TEST_F(StartInfoTest, SetUseShell_IndependentFromRedirectFlags) {
	// Arrange
	StartInfo info;

	// Act
	info.use_shell() = true;

	// Assert: use_shell だけが true、リダイレクトフラグは変化しない
	EXPECT_TRUE(info.use_shell());
	EXPECT_FALSE(info.redirect_standard_output());
	EXPECT_FALSE(info.redirect_standard_error());
}
