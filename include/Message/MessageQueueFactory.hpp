#ifndef INCLUDE_MESSAGE_MESSAGEQUEUEFACTORY_HPP
#define INCLUDE_MESSAGE_MESSAGEQUEUEFACTORY_HPP

#include "IMessageQueue.hpp"
#include "SynchronizedDeque.hpp"

namespace Framework::Message {
	class MessageQueueFactory final {
	public:
		template <typename T>
		static IMessageQueue<T> *create() {
			return new SynchronizedDeque<T>();
		}
	};
} // namespace Framework::Message

#endif // INCLUDE_MESSAGE_MESSAGEQUEUEFACTORY_HPP
