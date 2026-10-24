#include <condition_variable>
#include <iostream>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <type_traits>

namespace xstd {
	template <typename T> class threadsafe_queue {
	private:
		std::queue<T> m_queue;
		mutable std::mutex m_mutex;
		std::condition_variable not_empty_;

	public:
		using value_type = T;
		using reference = T &;
		using const_reference = const T &;

		threadsafe_queue() = default;

		threadsafe_queue(const threadsafe_queue &other) {
			std::unique_lock<std::mutex> unique_lck(other.m_mutex);
			m_queue = other.m_queue;
		}

		threadsafe_queue &operator=(const threadsafe_queue &) = delete;

		value_type front() {
			std::unique_lock<std::mutex> unique_lck(m_mutex);
			return m_queue.front();
		}

		value_type back() {
			std::unique_lock<std::mutex> unique_lck(m_mutex);
			return m_queue.back();
		}

		bool empty() {
			std::unique_lock<std::mutex> unique_lck(m_mutex);
			return m_queue.empty();
		}

		std::size_t size() {
			std::unique_lock<std::mutex> unique_lck(m_mutex);
			return m_queue.size();
		}

		// non-blocking
		bool try_push(const_reference item) {
			std::unique_lock<std::mutex> unique_lck(m_mutex, std::try_to_lock);
			if (!unique_lck)
				return false;

			if constexpr (std::is_nothrow_move_constructible_v<T>) {
				m_queue.push(std::move(item));
			} else {
				m_queue.push(item);
			}
			unique_lck.unlock();
			not_empty_.notify_one();
			return true;
		}

		// blocking
		void push(const_reference item) {
			std::unique_lock<std::mutex> unique_lck(m_mutex);
			m_queue.push(item);
			unique_lck.unlock();
			not_empty_.notify_one();
		}

		// non-blocking
		std::optional<T> try_pop() {
			std::unique_lock<std::mutex> unique_lck(m_mutex, std::try_to_lock);
			if (!unique_lck || m_queue.empty())
				return std::nullopt;

			value_type item;
			if constexpr (std::is_nothrow_move_assignable_v<T>) {
				item = std::move(m_queue.front());
			} else {
				item = m_queue.front();
			}
			m_queue.pop();
			return item;
		}

		// blocking
		value_type pop() {
			std::unique_lock<std::mutex> unique_lck(m_mutex);
			not_empty_.wait(unique_lck, [this]() { return !m_queue.empty(); });

			value_type item;
			if constexpr (std::is_nothrow_move_assignable_v<T>) {
				item = std::move(m_queue.front());
			} else {
				item = m_queue.front();
			}
			m_queue.pop();
			return item;
		}

		// blocking
		template <typename... Args> void emplace(Args &&...args) {
			std::unique_lock<std::mutex> unique_lck(m_mutex);
			m_queue.emplace(std::forward<Args>(args)...);
			unique_lck.unlock();
			not_empty_.notify_one();
		}
	};
} // namespace xstd