# CppFramework

C++20 ヘッダ主体の並行処理フレームワーク。

## 主な機能

- **Task System** — イベント駆動タスク (`MessageTask`, `StatementTask`)、タスク管理 (`EventTaskManager`)、バックグラウンドワーカー、スレッドプール
- **Message** — スレッドセーフキュー (`SynchronizedDeque`, `PosixMessageQueue`)
- **Sync** — ビットパターン待機フラグ (`EventFlag`)
- **SubProcess** — 子プロセス実行・制御 (`Process`, `ProcessSpawner`)
- **Timer** — ワンショット / 周期タイマー
- **Templates** — 型安全 Enum ビットセット、C# スタイルプロパティ

## 要件

- GCC / Clang（C++20 対応）、Linux（POSIX 環境）
- CMake 3.20+、Ninja
- GoogleTest（CMake により自動フェッチ）

## ビルド

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build && ctest
```

