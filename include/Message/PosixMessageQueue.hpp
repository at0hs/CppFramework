#ifndef INCLUDE_MESSAGE_POSIXMESSAGEQUEUE_HPP
#define INCLUDE_MESSAGE_POSIXMESSAGEQUEUE_HPP

#include "Exception/SystemException.hpp"
#include "IMessageQueue.hpp"

#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <mqueue.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace Framework::Message {

	// --- 所有権タグ ---
	struct Owner {};

	struct Attach {};

	/**
	 * POSIX メッセージキューを用いたプロセス間通信用キュー。
	 *
	 * - T は trivially copyable であること（Concept 制約）
	 * - Owner モード: キューを新規作成し、デストラクタで mq_unlink する (RAII)
	 * - Attach モード: 既存キューに接続のみ（主に送信者側）
	 */
	template <typename T>
		requires std::is_trivially_copyable_v<T>
	class PosixMessageQueue : public IMessageQueue<T> {
		static constexpr std::size_t kMsgSize = sizeof(T);

		mqd_t mqd_{ static_cast<mqd_t>(-1) };
		std::string name_;
		bool owner_{ false };

		static void validate_name(std::string_view name) {
			if (name.empty() || name[0] != '/') {
				throw Framework::Exception("PosixMessageQueue: queue name must start with '/'",
										   Error::Code::InvalidArgument);
			}
		}

		static timespec to_abs_timespec(std::chrono::milliseconds ms) {
			struct timespec ts{};
			clock_gettime(CLOCK_REALTIME, &ts);
			const auto extra_ns = ms.count() * 1'000'000LL;
			ts.tv_nsec += extra_ns % 1'000'000'000LL;
			ts.tv_sec += (ms.count() / 1000) + (ts.tv_nsec / 1'000'000'000LL);
			ts.tv_nsec %= 1'000'000'000LL;
			return ts;
		}

		void do_send(const T &message) {
			std::array<char, kMsgSize> buf;
			std::memcpy(buf.data(), &message, kMsgSize);
			if (mq_send(mqd_, buf.data(), kMsgSize, 0) < 0) {
				throw Framework::SystemException("PosixMessageQueue::send failed", errno);
			}
		}

	public:
		// --- Owner コンストラクタ ---
		PosixMessageQueue(std::string_view name, Owner /*tag*/, std::size_t max_msg = 10)
			: name_(name),
			  owner_(true) {
			validate_name(name);
			struct mq_attr attr{};
			attr.mq_maxmsg = static_cast<long>(max_msg);
			attr.mq_msgsize = static_cast<long>(kMsgSize);

			mqd_ = mq_open(name_.c_str(), O_CREAT | O_RDWR, 0666, &attr);
			if (mqd_ == static_cast<mqd_t>(-1)) {
				throw Framework::SystemException("PosixMessageQueue: mq_open(O_CREAT) failed", errno);
			}
		}

		// --- Attach コンストラクタ ---
		PosixMessageQueue(std::string_view name, Attach /*tag*/) : name_(name) {
			validate_name(name);
			mqd_ = mq_open(name_.c_str(), O_RDWR);
			if (mqd_ == static_cast<mqd_t>(-1)) {
				throw Framework::SystemException("PosixMessageQueue: mq_open(attach) failed", errno);
			}
		}

		~PosixMessageQueue() {
			if (mqd_ != static_cast<mqd_t>(-1)) {
				mq_close(mqd_);
			}
			if (owner_) {
				mq_unlink(name_.c_str());
			}
		}

		// コピー・ムーブ禁止（mqd_t の所有権管理のため）
		PosixMessageQueue(const PosixMessageQueue &) = delete;
		PosixMessageQueue &operator=(const PosixMessageQueue &) = delete;
		PosixMessageQueue(PosixMessageQueue &&) = delete;
		PosixMessageQueue &operator=(PosixMessageQueue &&) = delete;

		void send(const T &message) override { do_send(message); }

		void send(T &&message) override { do_send(std::move(message)); }

		T receive() override {
			std::array<char, kMsgSize> buf;
			const auto ret = mq_receive(mqd_, buf.data(), kMsgSize, nullptr);
			if (ret < 0) {
				throw Framework::SystemException("PosixMessageQueue::receive failed", errno);
			}
			T result;
			std::memcpy(&result, buf.data(), kMsgSize);
			return result;
		}

		std::pair<bool, T> timed_receive(std::chrono::milliseconds milli_sec) override {
			const auto ts = to_abs_timespec(milli_sec);
			std::array<char, kMsgSize> buf;
			const auto ret = mq_timedreceive(mqd_, buf.data(), kMsgSize, nullptr, &ts);
			if (ret < 0) {
				if (errno == ETIMEDOUT) {
					return { false, T{} };
				}
				throw Framework::SystemException("PosixMessageQueue::timed_receive failed", errno);
			}
			T result;
			std::memcpy(&result, buf.data(), kMsgSize);
			return { true, result };
		}

		bool is_empty() const override { return num_message() == 0; }

		void clear() override {
			// 一時的に O_NONBLOCK を設定してドレイン
			struct mq_attr nonblock_attr{};
			struct mq_attr old_attr{};
			nonblock_attr.mq_flags = O_NONBLOCK;
			mq_setattr(mqd_, &nonblock_attr, &old_attr);

			std::array<char, kMsgSize> buf;
			while (mq_receive(mqd_, buf.data(), kMsgSize, nullptr) >= 0) {}

			mq_setattr(mqd_, &old_attr, nullptr);
		}

		std::size_t num_message() const override {
			struct mq_attr attr{};
			if (mq_getattr(mqd_, &attr) < 0) {
				throw Framework::SystemException("PosixMessageQueue::num_message failed", errno);
			}
			return static_cast<std::size_t>(attr.mq_curmsgs);
		}
	};
} // namespace Framework::Message

#endif // INCLUDE_MESSAGE_POSIXMESSAGEQUEUE_HPP
