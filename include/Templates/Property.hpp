#ifndef INCLUDE_TEMPLATES_PROPERTY_HPP
#define INCLUDE_TEMPLATES_PROPERTY_HPP
#include <functional>

namespace Framework::Templates {

	class ReferenceProperty {
		template <typename T>
		class Base {
		protected:
			std::reference_wrapper<T> value_;

		public:
			Base(T &value) : value_(value) {}

			virtual ~Base() = default;

			Base(const Base &) = default;
			Base(Base &&) = default;
			Base &operator=(const Base &) = default;
			Base &operator=(Base &&) = default;
		};

	public:
		template <typename T>
		class DefaultGetter {
		public:
			static const T &get(const T &value) { return value; }

			static const T &get(T &&) = delete;
		};

		template <typename T>
		class DefaultSetter {
		public:
			static void set(T &value, const T &new_value) { value = new_value; }
		};

		template <typename T, typename Getter = DefaultGetter<T>>
		class ReadOnly : public Base<const T> {
		public:
			ReadOnly(T &value) : Base<const T>(value) {}

			ReadOnly(const T &value) : Base<const T>(value) {}

			operator auto() const { return Getter::get(this->value_.get()); }
		};

		template <typename T, typename Getter = DefaultGetter<T>,
				  typename Setter = DefaultSetter<T>>
		class Writable : public ReadOnly<T, Getter> {
			std::reference_wrapper<T> mutable_ref_;

		public:
			Writable(T &value) : ReadOnly<T, Getter>(value), mutable_ref_(value) {}

			auto &operator=(const T &value) {
				Setter::set(mutable_ref_.get(), value);
				return *this;
			}
		};

		template <typename T>
		class FunctionSetter : public Base<T> {
		public:
			using Type = T;

			FunctionSetter(Type &value) : Base<Type>(value) {}

			template <typename F>
			auto &operator=(F &&value) {
				this->value_.get() = std::forward<F>(value);
				return *this;
			}
		};

		template <typename Ret, typename... ArgTypes>
		class FunctionSetter<Ret(ArgTypes...)> : public Base<std::function<Ret(ArgTypes...)>> {
		public:
			using Type = std::function<Ret(ArgTypes...)>;

			FunctionSetter(Type &value) : Base<Type>(value) {}

			template <typename F>
			auto &operator=(F &&value) {
				this->value_.get() = std::forward<F>(value);
				return *this;
			}
		};
	};

	template <typename T>
	class Property {
	public:
		Property() = delete;
		~Property() = delete;

		Property(const Property &) = delete;
		Property(Property &&) = delete;
		Property &operator=(const Property &) = delete;
		Property &operator=(Property &&) = delete;

		using SetterType = std::function<void(const T &)>;
		using GetterType = std::function<T()>;

		class ReadOnly {
		protected:
			GetterType getter_;

		public:
			ReadOnly(GetterType getter) : getter_(getter) {}

			operator T() const { return getter_(); }
		};

		class Writable : public ReadOnly {
		protected:
			SetterType setter_;

		public:
			Writable(GetterType getter, SetterType setter) : ReadOnly(getter), setter_(setter) {}

			auto &operator=(const T &value) {
				setter_(value);
				return *this;
			}
		};
	};
} // namespace Framework::Templates
#endif // INCLUDE_TEMPLATES_PROPERTY_HPP
