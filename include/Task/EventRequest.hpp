#ifndef INCLUDE_TASK_EVENTREQUEST_HPP
#define INCLUDE_TASK_EVENTREQUEST_HPP

#include <any>
#include <cstdint>
#include <string>
#include <utility>

namespace Framework::Task {

	template <typename T = int64_t>
	class EventRequest {
	public:
		using Command = T;

	private:
		std::string from_;
		Command command_{ 0 };
		std::any payload_;

	public:
		EventRequest() = default;
		EventRequest(std::string from, Command command, std::any payload)
			: from_(std::move(from)),
			  command_(command),
			  payload_(std::move(payload)) {}
		EventRequest(std::string from, Command command)
			: from_(std::move(from)),
			  command_(command) {}
		EventRequest(const EventRequest &other) : from_(other.from_), command_(other.command_) {
			if (other.has_payload()) {
				payload_ = other.payload_;
			}
		}
		EventRequest(EventRequest &&other) noexcept
			: from_(std::move(other.from_)),
			  command_(other.command_),
			  payload_(std::move(other.payload_)) {}

		~EventRequest() = default;

		EventRequest &operator=(EventRequest &&other) noexcept {
			if (this != &other) {
				from_ = std::move(other.from_);
				command_ = other.command_;
				payload_ = std::move(other.payload_);
			}
			return *this;
		}

		EventRequest &operator=(const EventRequest &other) {
			if (this != &other) {
				from_ = other.from_;
				command_ = other.command_;
				if (other.has_payload()) {
					payload_ = other.payload_;
				}
			}
			return *this;
		}

		const std::string &get_from() const { return from_; }

		Command get_command() const { return command_; }

		bool has_payload() const { return payload_.has_value(); }
		const std::any &get_payload() const { return payload_; }
		template <typename U>
		const U &get_payload_as() const {
			return std::any_cast<const U &>(payload_);
		}
	};
} // namespace Framework::Task

#endif // INCLUDE_TASK_EVENTREQUEST_HPP
