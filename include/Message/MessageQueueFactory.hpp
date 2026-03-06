#ifndef INCLUDE_MESSAGE_MESSAGEQUEUEFACTORY_HPP
#define INCLUDE_MESSAGE_MESSAGEQUEUEFACTORY_HPP

#include "IMessageQueue.hpp"
#include "SynchronizedDeque.hpp"
#include <memory>

namespace Framework::Message {
	class MessageQueueFactory final {
	public:
		template <typename T>
		static std::shared_ptr<IMessageQueue<T>> create() {
			return std::make_shared<SynchronizedDeque<T>>();
		}
	};
} // namespace Framework::Message

#endif // INCLUDE_MESSAGE_MESSAGEQUEUEFACTORY_HPP
