# CppFramework

C++20 で実装されたヘッダ主体の並行処理フレームワークライブラリです。
イベント駆動タスク、状態機械、スレッドプール、プロセス管理など、
組み込み・サーバーアプリケーションで必要となる並行処理の基盤を提供します。

## 主な機能

| カテゴリ | 機能 |
|---------|------|
| **Task System** | コマンド駆動タスク (`MessageTask`)、状態機械タスク (`StatementTask`)、バックグラウンドワーカー (`BackGroundWorker`)、スレッドプール (`TaskPool`)、タスクマネージャー (`EventTaskManager`) |
| **Message** | スレッドセーフキュー (`SynchronizedDeque`)、POSIX メッセージキュー (`PosixMessageQueue`) |
| **Sync** | ビットパターン待機フラグ (`EventFlag`) |
| **SubProcess** | 子プロセス実行・制御 (`Process`)、フォーク (`ProcessSpawner`) |
| **Timer** | ワンショット / 周期タイマー (`Timer`) |
| **Templates** | 型安全 Enum ビットセット (`EnumBitset`)、Enum ビット演算 (`EnumOperations`)、C# スタイルプロパティ (`Property`) |
| **Exception** | エラーコード付き例外 (`Exception`, `SystemException`) |

## 要件

- **コンパイラ**: GCC / Clang（C++20 対応）
- **OS**: Linux（POSIX 環境）
- **ビルドツール**: CMake 3.20+、Ninja
- **テスト**: GoogleTest（CMake により自動フェッチ）
- **カバレッジ**: lcov / genhtml（オプション）

## ビルド

```bash
# 初回設定（Debug ビルド）
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# ビルド
cmake --build build

# 特定ターゲットのみビルド
cmake --build build --target TaskTest

# テスト実行（全スイート）
cd build && ctest

# テストバイナリを直接実行
./build/tests/TaskTest/TaskTest
```

### ビルド構成

| 構成 | 説明 |
|------|------|
| `Debug`（デフォルト） | `-O0 -ggdb -g3` |
| `TestPlus` | AddressSanitizer + gcov カバレッジ |

```bash
# TestPlus ビルド（AddressSanitizer + カバレッジ）
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=TestPlus
cmake --build build

# カバレッジレポート生成（lcov / genhtml が必要）
python3 scripts/gencov.py TaskTest
```

## クイックスタート

### コマンド駆動タスク（MessageTask）

コマンド enum を定義し、ハンドラを登録するだけでスレッドセーフなメッセージポンプが動作します。

```cpp
#include "Task/MessageTask.hpp"
#include "Task/EventRequest.hpp"

using namespace Framework::Task;

enum class Command : uint8_t { Hello = 0, Shutdown = 1 };

int main() {
    // ハンドラマップを定義
    const MessageTask<Command>::EventMap events{
        { Command::Hello,    { [](const EventRequest<Command>& req) {
            // req.get_payload_as<std::string>() でペイロード取得
            return true;
        }}},
        { Command::Shutdown, { [](const EventRequest<Command>&) {
            return true;
        }}},
    };

    MessageTask<Command> task("my-task", events);
    task.start();

    // 非同期（fire-and-forget）
    task.send_event({ "main", Command::Hello, std::string("world") });

    // 同期 RPC（ハンドラ完了まで待機）
    bool ok = task.rpc_event({ "main", Command::Shutdown });

    task.stop();
}
```

### 状態機械タスク（StatementTask）

ビルダー API で状態遷移を直感的に定義できます。

```cpp
#include "Task/StatementTask.hpp"

using namespace Framework::Task;

enum class State   : uint8_t { Idle, Running };
enum class Command : uint8_t { Start, Stop };

int main() {
    StatementTask<State, Command> task("fsm-task", State::Idle);

    task.on(State::Idle,    Command::Start, [](const EventRequest<Command>&) { return true; }, State::Running)
        .on(State::Running, Command::Stop,  [](const EventRequest<Command>&) { return true; }, State::Idle);

    task.state_changed = [](State from, State to) {
        // 状態遷移時のコールバック
    };

    task.start();
    task.rpc_event({ "main", Command::Start });
    // task.get_state() == State::Running
    task.stop();
}
```

## ドキュメント

詳細なドキュメントは [`private-repo/`](private-repo/) を参照してください。

| ドキュメント | 内容 |
|-------------|------|
| [設計書](private-repo/design.md) | 継承ツリー、クラス図、コンポーネント依存関係 |
| [使用ガイド](private-repo/usage-guide.md) | 各機能のユースケースとコード例 |
| [API リファレンス](private-repo/api-reference.md) | 全クラスのシグネチャ一覧 |
| [アーキテクチャ決定記録](private-repo/architecture-decisions.md) | 設計判断の背景と理由 |

## ディレクトリ構成

```
CppFramework/
├── include/          # 公開 API（ヘッダオンリー中心）
│   ├── Task/         # タスクシステム
│   ├── Message/      # メッセージキュー
│   ├── Sync/         # 同期プリミティブ
│   ├── SubProcess/   # プロセス管理
│   ├── Templates/    # 型ユーティリティ
│   ├── Exception/    # 例外
│   ├── Main/         # 設定・ワークスペース
│   └── Timer.hpp     # タイマー
├── lib/              # 実装 .cpp
├── tests/            # GoogleTest テストスイート
└── scripts/          # ユーティリティスクリプト
```

## テストスイート

| スイート | カバー内容 |
|---------|----------|
| `TaskTest` | MessageTask, StatementTask, BackGroundWorker, TaskPool, EventTaskManager |
| `SyncTest` | EventFlag |
| `MessageTest` | SynchronizedDeque, PosixMessageQueue |
| `TemplatesTest` | EnumBitset, EnumOperations, Property |
| `TimerTest` | Timer |
| `SubProcessTest` | Process, StartInfo, ProcessSpawner |
