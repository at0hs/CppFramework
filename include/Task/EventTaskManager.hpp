#ifndef INCLUDE_TASK_EVENTTASKMANAGER_HPP
#define INCLUDE_TASK_EVENTTASKMANAGER_HPP

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Exception/Exception.hpp"
#include "Task/EventRequest.hpp"
#include "Task/interface/IEventTask.hpp"

namespace Framework::Task {

	/// @brief イベントタスク（MessageTask / StatementTask）を名前で管理し、
	///        タスクのインスタンスを直接知らなくてもメッセージをルーティングできる管理クラス。
	///
	/// 同一コマンド型 CommandType を持つタスクを一つの Manager で管理する。
	/// 異なるコマンド型を持つタスクは別々の EventTaskManager インスタンスで管理すること。
	///
	/// スレッドセーフ: register_task / unregister_task は write ロック、
	///                 send_event / rpc_event / クエリ系は read ロックで保護。
	template <typename CommandType>
		requires(std::is_integral_v<CommandType> || std::is_enum_v<CommandType>)
	class EventTaskManager {
	public:
		using Request = EventRequest<CommandType>;
		using TaskPtr = std::shared_ptr<IEventTask<CommandType>>;

		EventTaskManager() = default;

		~EventTaskManager() { tasks_.clear(); }

		// コピー・ムーブ不可（mutex を所有しているため）
		EventTaskManager(const EventTaskManager &) = delete;
		EventTaskManager &operator=(const EventTaskManager &) = delete;
		EventTaskManager(EventTaskManager &&) = delete;
		EventTaskManager &operator=(EventTaskManager &&) = delete;

		/// @brief タスクを名前で登録する。
		/// @param name   登録名（タスクの識別子）
		/// @param task   登録するタスクの shared_ptr
		/// @throws Framework::Exception (InvalidArgument) task が nullptr の場合
		void register_task(std::string name, TaskPtr task) {
			if (!task) {
				throw Framework::Exception("register_task: task must not be null",
										   Framework::Error::Code::InvalidArgument);
			}
			std::unique_lock lock(mutex_);
			tasks_.insert_or_assign(std::move(name), std::move(task));
		}

		/// @brief 名前でタスクを登録解除する。存在しない場合は何もしない。
		void unregister_task(const std::string &name) {
			std::unique_lock lock(mutex_);
			tasks_.erase(name);
		}

		/// @brief 指定した名前のタスクが登録されているか確認する。
		[[nodiscard]] bool has_task(const std::string &name) const {
			std::shared_lock lock(mutex_);
			return tasks_.contains(name);
		}

		/// @brief 登録されているタスク名の一覧を返す。
		[[nodiscard]] std::vector<std::string> task_names() const {
			std::shared_lock lock(mutex_);
			std::vector<std::string> names;
			names.reserve(tasks_.size());
			for (const auto &[name, _] : tasks_) {
				names.push_back(name);
			}
			return names;
		}

		/// @brief 名前でタスクの shared_ptr を取得する。
		/// @throws Framework::Exception (InvalidArgument) 指定した名前のタスクが存在しない場合
		[[nodiscard]] TaskPtr get_task(const std::string &name) const {
			std::shared_lock lock(mutex_);
			return find_or_throw(name);
		}

		/// @brief 名前を指定して fire-and-forget でイベントを送信する（右辺値参照版）。
		/// @throws Framework::Exception (InvalidArgument) 指定した名前のタスクが存在しない場合
		void send_event(const std::string &name, Request &&request) {
			std::shared_lock lock(mutex_);
			find_or_throw(name)->send_event(std::move(request));
		}

		/// @brief 名前を指定して fire-and-forget でイベントを送信する（const 左辺値参照版）。
		/// @throws Framework::Exception (InvalidArgument) 指定した名前のタスクが存在しない場合
		void send_event(const std::string &name, const Request &request) {
			std::shared_lock lock(mutex_);
			find_or_throw(name)->send_event(request);
		}

		/// @brief 名前を指定して同期 RPC イベントを送信する（右辺値参照版）。
		/// @param timeout タイムアウト（kWaitForever = ゼロ で無限待機）
		/// @return ハンドラの戻り値。タイムアウト時は false。
		/// @throws Framework::Exception (InvalidArgument) 指定した名前のタスクが存在しない場合
		bool rpc_event(const std::string &name, Request &&request,
					   std::chrono::milliseconds timeout = IEventTask<CommandType>::kWaitForever) {
			std::shared_lock lock(mutex_);
			return find_or_throw(name)->rpc_event(std::move(request), timeout);
		}

		/// @brief 名前を指定して同期 RPC イベントを送信する（const 左辺値参照版）。
		/// @param timeout タイムアウト（kWaitForever = ゼロ で無限待機）
		/// @return ハンドラの戻り値。タイムアウト時は false。
		/// @throws Framework::Exception (InvalidArgument) 指定した名前のタスクが存在しない場合
		bool rpc_event(const std::string &name, const Request &request,
					   std::chrono::milliseconds timeout = IEventTask<CommandType>::kWaitForever) {
			std::shared_lock lock(mutex_);
			return find_or_throw(name)->rpc_event(request, timeout);
		}

	private:
		using TaskMap = std::unordered_map<std::string, TaskPtr>;

		mutable std::shared_mutex mutex_;
		TaskMap tasks_;

		/// ロック取得済みの状態で名前でタスクを検索し、なければ例外を投げる。
		[[nodiscard]] TaskPtr find_or_throw(const std::string &name) const {
			const auto it = tasks_.find(name);
			if (it == tasks_.end()) {
				throw Framework::Exception("EventTaskManager: task not found: " + name,
										   Framework::Error::Code::InvalidArgument);
			}
			return it->second;
		}
	};

} // namespace Framework::Task

#endif // INCLUDE_TASK_EVENTTASKMANAGER_HPP
