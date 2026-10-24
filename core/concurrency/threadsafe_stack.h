#include <mutex>
#include <optional>
#include <shared_mutex>
#include <stack>

namespace xstd {
	template <typename T> class threadsafe_stack {
	private:
		std::stack<T> m_stack;
		mutable std::shared_mutex m_shared_mutex;

	public:
		using value_type = T;
		using reference = T &;
		using const_reference = const T &;

		// default constructor
		threadsafe_stack() = default;

		// copy constructor
		threadsafe_stack(const threadsafe_stack &other) {
			std::shared_lock<std::shared_mutex> shared_lck((other.m_shared_mutex));
			m_stack = other.m_stack;
		}

		// copy assignment
		threadsafe_stack &operator=(const threadsafe_stack &) = delete;

		/**
		 * @brief Inserts an element at the top of the stack.
		 */
		void push(const_reference element) {
			std::unique_lock unique_lck(m_shared_mutex);
			if constexpr (std::is_nothrow_constructible_v<T>) {
				m_stack.push(std::move(element));
			} else {
				m_stack.push(element);
			}
		}

		std::optional<T> pop() {
			std::unique_lock<std::shared_mutex> unique_lck(m_shared_mutex);
			if (m_stack.empty())
				return std::nullopt;

			T element;
			if constexpr (std::is_nothrow_move_constructible_v<T>) {
				element = std::move(m_stack.top());
			} else {
				element = m_stack.top();
			}
			m_stack.pop();
			return element;
		}

		value_type top() {
			std::shared_lock<std::shared_mutex> shared_lck(m_shared_mutex);
			return m_stack.top();
		}

		bool empty() {
			std::shared_lock<std::shared_mutex> shared_lck(m_shared_mutex);
			return m_stack.empty();
		}

		std::size_t size() {
			std::shared_lock<std::shared_mutex> shared_lck(m_shared_mutex);
			return m_stack.size();
		}

		void swap(threadsafe_stack &other) {
			std::scoped_lock<std::shared_mutex, std::shared_mutex> scoped_lck(m_shared_mutex, other.m_shared_mutex);
			std::swap(m_stack, other.m_stack);
		}

		friend void swap(threadsafe_stack &lhs, threadsafe_stack &rhs) { lhs.swap(rhs); }

		friend bool operator==(threadsafe_stack &lhs, threadsafe_stack &rhs) {
			std::scoped_lock<std::shared_mutex, std::shared_mutex> scoped_lck(lhs.m_shared_mutex, rhs.m_shared_mutex);
			if (lhs.m_stack.size() != rhs.m_stack.size())
				return false;
			return lhs.m_stack == rhs.m_stack;
		}
	};
} // namespace xstd