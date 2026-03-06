#ifndef INCLUDE_TASK_EVENTTASKBASE_HPP
#define INCLUDE_TASK_EVENTTASKBASE_HPP

#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <thread>

#include "Task/EventRequest.hpp"
#include "Task/TaskBase.hpp"
#include "Task/interface/IEventTask.hpp"

#include "Message/IMessageQueue.hpp"
#include "Message/MessageQueueFactory.hpp"

namespace Framework::Task {
	using namespace Framework;

	template <typename T, typename Aggregator>
	class EventTaskBase : public IEventTask<T>, TaskBase {

	private:
		using Request = EventRequest<T>;
		using Command = Request::Command;
		static constexpr std::chrono::milliseconds kWaitForever = IEventTask<T>::kWaitForever;

		class Attribute final {
		public:
			enum class Type : uint8_t {
				None = 0,
				Internal,
				External,
			};

		private:
			Type type_{ Type::None };

		public:
			Attribute() = default;
			explicit Attribute(Type flags) : type_(flags) {}
			bool is_internal() const { return type_ == Type::Internal; }
			bool is_external() const { return type_ == Type::External; }

			static Attribute internal() { return Attribute(Type::Internal); }
			static Attribute external() { return Attribute(Type::External); }
		};

		class Response final {
			std::promise<bool> response_;

		public:
			std::future<bool> get_future() { return response_.get_future(); }

			void set(bool response) { response_.set_value(response); }
			void handle_exception() { response_.set_exception(std::current_exception()); }
		};

		class InternalCommands final {
		public:
			static constexpr EventRequest<>::Command kStart = 0;
			static constexpr EventRequest<>::Command kStop = 1;
		};

		class MessageContent {
			Attribute attribute_{};
			Request request_{};
			std::shared_ptr<Response> response_{ nullptr };

		public:
			MessageContent() = default;
			MessageContent(Attribute attribute, const Request &request,
						   std::shared_ptr<Response> response = nullptr)
				: attribute_(attribute),
				  request_(request),
				  response_(response) {}
			MessageContent(Attribute attribute, Request &&request,
						   std::shared_ptr<Response> response = nullptr)
				: attribute_(attribute),
				  request_(std::move(request)),
				  response_(response) {}

			const auto &get_attribute() const { return attribute_; }
			const auto &get_request() const { return request_; }
			auto &get_response_buffer() const { return response_; }
			bool is_response_required() const { return response_ != nullptr; }
		};

		using MessageQueue = Message::IMessageQueue<MessageContent>;

		class Sender {
			std::weak_ptr<MessageQueue> message_queue_;
			std::shared_ptr<Response> response_{ nullptr };
			bool sent_{ false };

		public:
			Sender(const std::shared_ptr<MessageQueue> &message_queue, bool need_response = false)
				: message_queue_(message_queue),
				  response_(need_response ? std::make_shared<Response>() : nullptr) {}

			void send(Attribute attribute, const Request &request) {
				if (auto message_queue = message_queue_.lock()) {
					message_queue->send({ attribute, request, response_ });
					sent_ = true;
				}
			}

			bool wait_for_response(std::chrono::milliseconds timeout_msec = kWaitForever) {
				if (!response_ || !sent_) {
					return false;
				}
				std::future<bool> future = response_->get_future();
				if (timeout_msec == kWaitForever) {
					future.wait();
				} else {
					if (future.wait_for(timeout_msec) == std::future_status::timeout) {
						return false;
					}
				}
				return future.get();
			}
		};

		Aggregator aggregator_;
		std::shared_ptr<MessageQueue> message_queue_;

		std::function<void()> on_start_;
		std::function<void()> on_finish_;
		bool stopped_ = false;

	public:
		template <typename... AggregatorArgs>
		EventTaskBase(TaskType type, const std::string &name, AggregatorArgs &&...args)
			: TaskBase(type, name),
			  aggregator_(std::forward<AggregatorArgs>(args)...),
			  message_queue_(Message::MessageQueueFactory::create<MessageContent>()) {
			thread_ = std::thread([this]() { main_loop(); }); // NOLINT(cppcoreguidelines-prefer-member-initializer)
		}

		~EventTaskBase() { stop(); }

		EventTaskBase(const EventTaskBase &) = delete;
		EventTaskBase &operator=(const EventTaskBase &) = delete;
		EventTaskBase(EventTaskBase &&) = delete;
		EventTaskBase &operator=(EventTaskBase &&) = delete;

		void start() override {
			Sender sender{ message_queue_, true };
			sender.send(Attribute::internal(),
						Request{ "", static_cast<T>(InternalCommands::kStart) });
			sender.wait_for_response();
		}

		void stop() override {
			if (!is_running()) {
				return;
			}
			Sender sender{ message_queue_, true };
			sender.send(Attribute::internal(),
						Request{ "", static_cast<T>(InternalCommands::kStop) });
			sender.wait_for_response();
			thread_.join();
		}

		void send_event(Request &&request) override {
			Sender sender(message_queue_, false);
			sender.send(Attribute::external(), std::move(request));
		}

		void send_event(const Request &request) override {
			Sender sender(message_queue_, false);
			sender.send(Attribute::external(), request);
		}

		bool rpc_event(Request &&request,
					   std::chrono::milliseconds timeout_msec = kWaitForever) override {
			Sender sender(message_queue_, true);
			sender.send(Attribute::external(), std::move(request));
			return sender.wait_for_response(timeout_msec);
		}

		bool rpc_event(const Request &request,
					   std::chrono::milliseconds timeout_msec = kWaitForever) override {
			Sender sender(message_queue_, true);
			sender.send(Attribute::external(), request);
			return sender.wait_for_response(timeout_msec);
		}

		void set_on_start(const std::function<void()> &on_start) { on_start_ = on_start; }

		void set_on_finish(const std::function<void()> &on_finish) { on_finish_ = on_finish; }

		bool is_running() const noexcept { return thread_.joinable(); }

	protected:
		Aggregator &aggregator() { return aggregator_; }
		const Aggregator &aggregator() const { return aggregator_; }

	private:
		void main_loop() {
			while (!stopped_) {
				MessageContent content = message_queue_->receive();
				try {
					bool response_value = true;
					if (content.get_attribute().is_internal()) {
						process_internal_command(content.get_request());
					} else {
						response_value = process_event(content.get_request());
					}
					if (content.is_response_required()) {
						content.get_response_buffer()->set(response_value);
					}
				} catch (...) {
					if (content.is_response_required()) {
						content.get_response_buffer()->handle_exception();
					}
				}
			}
			if (on_finish_) {
				on_finish_();
			}
		}

		void process_internal_command(const Request &request) {
			auto command = static_cast<EventRequest<>::Command>(request.get_command());
			switch (command) {
			case InternalCommands::kStart:
				if (on_start_) {
					on_start_();
				}
				break;
			case InternalCommands::kStop:
				stopped_ = true;
				break;
			default:
				break;
			}
		}

		bool process_event(const Request &request) {
			return aggregator_.publish(request.get_command(), request);
		}
	};
} // namespace Framework::Task
#endif // INCLUDE_TASK_EVENTTASKBASE_HPP
