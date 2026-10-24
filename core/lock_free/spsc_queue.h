#include <atomic>
#include <concepts>
#include <math.h>
#include <optional>
#include <type_traits>

namespace xstd {

	template <typename T>
	concept Queueable = std::default_initializable<T> && std::move_constructible<T>;

	/**
	 * @brief The `spsc_queue` class provides a single-reader,
	 * single-writer fifo queue.
	 */
	template <Queueable T, std::size_t N> class spsc_queue {
	private:
		using size_type = std::size_t;
		using value_type = T;
		using reference = T &;

		static constexpr std::size_t m_capacity{1 << N};
		T m_buffer[m_capacity];
		alignas(64) std::atomic<std::size_t> m_read_index{0};
		alignas(64) std::atomic<std::size_t> m_write_index{0};

	public:
		spsc_queue() = default;
		spsc_queue(const spsc_queue &) = delete;
		spsc_queue &operator=(const spsc_queue &) = delete;
		spsc_queue(spsc_queue &&) = delete;
		spsc_queue &operator=(spsc_queue &&) = delete;

		/**
		 * @brief pushes an element onto the ringbuffer.
		 * @param `element` will be pushed to the queue unless the queue is
		 * not full
		 */
		template <typename U>
		requires std::is_convertible_v<U, T>
		bool try_push(U &&element) {
			const std::size_t write_index = m_write_index.load(std::memory_order_relaxed);
			const std::size_t next_write_index = (write_index + 1) & (m_capacity - 1);

			if (next_write_index != m_read_index.load(std::memory_order_acquire)) {
				m_buffer[write_index] = std::forward<U>(element);
				m_write_index.store(next_write_index, std::memory_order_release);
				return true;
			}
			return false;
		}

		std::optional<T> try_pop() {
			std::optional<T> result{std::nullopt};
			const std::size_t read_index = m_read_index.load(std::memory_order_relaxed);

			if (read_index == m_write_index.load(std::memory_order_acquire))
				return result;

			result = std::move(m_buffer[read_index]);
			m_read_index.store((read_index + 1) & (m_capacity - 1), std::memory_order_release);
			return result;
		}
	};
} // namespace xstd