#include <concepts>
#include <utility>

// Compiler Explorer: https://compiler-explorer.com/z/var7onqaW
namespace xstd {

	/**
	 * @brief The default deleter for the managed object
	 */
	template <typename T> struct default_deleter {
		/**
		 * @brief Overloaded function call operator that calls delete on
		 * the @a raw_ptr
		 */
		void operator()(T *raw_ptr) { delete raw_ptr; }

		friend void swap(default_deleter &lhs, default_deleter &rhs) {}
	};

	/**
	 * @brief Default deleter when the pointee is an array of objects
	 */
	template <typename T> struct default_deleter<T[]> {
		void operator()(T *raw_ptr) { delete[] raw_ptr; }
		friend void swap(default_deleter &lhs, default_deleter &rhs) {}
	};

	// Single object version
	template <typename T, typename D = default_deleter<T>> class unique_ptr : public D {
	public:
		using deleter_type = D;
		using value_type = T;
		using pointer = T *;
		using const_pointer = const T *;
		using reference = T &;
		using const_reference = const T &;

		/**
		 * @brief Default constructor
		 */
		unique_ptr() : m_underlying_ptr{nullptr} {}

		// Copy c'tor and copy assignment are delete'd.
		unique_ptr(const unique_ptr &) = delete;
		unique_ptr &operator=(const unique_ptr &) = delete;

		/**
		 * @brief Constructor that takes a raw pointer
		 * @param ptr Pointer to the managed resource
		 */
		explicit unique_ptr(T *ptr) : m_underlying_ptr{ptr} {}

		/**
		 * @brief Swaps the two smart pointer objects member-by-member.
		 * @param other The other unique pointer instance
		 */
		void swap(unique_ptr &other) noexcept {
			std::swap(m_underlying_ptr, other.m_underlying_ptr);
			// To keep our code generic, we also swap the deleter, since
			// the deleter can be stateful. Consider for example:
			// ArenaAllocator arena{4096};
			// std::unique_ptr<Foo, ArenaAllocationDeleter{&arena}> fooPtr;
			// When we swap two unique pointers, you expect the
			// point-to-pool or arena to be swapped as well.
			deleter_type *this_deleter = static_cast<deleter_type *>(this);
			deleter_type *other_deleter = static_cast<deleter_type *>(&other);
			std::swap(this_deleter, other_deleter);
		}

		/**
		 * @brief Move constructor
		 */
		unique_ptr(unique_ptr &&other) noexcept
		    : m_underlying_ptr{std::exchange(other.m_underlying_ptr, nullptr)},
		      deleter_type{std::move(static_cast<deleter_type>(other))} {}

		/**
		 * @brief Move assignment operator
		 */
		unique_ptr &operator=(unique_ptr &&other) {
			unique_ptr(std::move(other)).swap(*this);
			return *this;
		}

		/**
		 * @brief Destructor
		 */
		~unique_ptr() {
			deleter_type *deleter = static_cast<deleter_type *>(this);
			(*deleter)(m_underlying_ptr);
		}

		/**
		 * @brief Dererencing operator
		 */
		[[nodiscard]] T operator*() { return *m_underlying_ptr; }

		/**
		 * @brief Indirection operator
		 */
		[[nodiscard]] T *operator->() { return m_underlying_ptr; }

		/**
		 * @brief Get the underlying raw pointer.
		 */
		[[nodiscard]] T *get() const { return m_underlying_ptr; }

		// Modifiers
		/**
		 * @brief Releases the underlying managed resource
		 */
		[[nodiscard]] T *release() { return std::exchange(m_underlying_ptr, nullptr); }

		/**
		 * @brief Replaces the managed object
		 */
		void reset(T *other) {
			if (m_underlying_ptr != other) {
				(*static_cast<deleter_type *>(this))(m_underlying_ptr);
				m_underlying_ptr = other;
			}
		}

		/**
		 * @brief Implementation of operator bool()
		 */
		explicit operator bool() const { return m_underlying_ptr == nullptr; }

		/**
		 * @brief Implementation of the spaceship operator
		 */
		friend auto operator<=>(const unique_ptr &lhs, const unique_ptr &rhs) { return lhs.get() <=> rhs.get(); }

		/**
		 * @brief Compare a unique_ptr will nullptr
		 */
		friend bool operator==(const unique_ptr &lhs, std::nullptr_t) { return lhs.get() == nullptr; }

		/**
		 * @brief Compare a nullptr will unique_ptr
		 */
		friend bool operator==(std::nullptr_t, const unique_ptr &rhs) { return rhs.get() == nullptr; }

		/**
		 * @brief std::swap called on two unique_ptr objects
		 */
		friend void swap(unique_ptr &lhs, unique_ptr &rhs) noexcept { lhs.swap(rhs); }

	private:
		T *m_underlying_ptr;
	};

	/* Array version*/
	template <typename T, typename D> class unique_ptr<T[], D> : public D {
	public:
		using deleter_type = D;
		using value_type = T;
		using pointer = T *;
		using const_pointer = const T *;
		using reference = T &;
		using const_reference = const T &;

		/**
		 * @brief Default constructor
		 */
		unique_ptr() : m_underlying_ptr{nullptr} {}

		// Copy c'tor and copy assignment are delete'd.
		unique_ptr(const unique_ptr &) = delete;
		unique_ptr &operator=(const unique_ptr &) = delete;

		/**
		 * @brief Constructor that takes a raw pointer
		 * @param ptr Pointer to the managed resource
		 */
		explicit unique_ptr(T *ptr) : m_underlying_ptr{ptr} {}

		/**
		 * @brief Swaps the two smart pointer objects member-by-member.
		 * @param other The other unique pointer instance
		 */
		void swap(unique_ptr &other) noexcept {
			std::swap(m_underlying_ptr, other.m_underlying_ptr);
			// To keep our code generic, we also swap the deleter, since
			// the deleter can be stateful. Consider for example:
			// ArenaAllocator arena{4096};
			// std::unique_ptr<Foo, ArenaAllocationDeleter{&arena}> fooPtr;
			// When we swap two unique pointers, you expect the
			// point-to-pool or arena to be swapped as well.
			deleter_type *this_deleter = static_cast<deleter_type *>(this);
			deleter_type *other_deleter = static_cast<deleter_type *>(&other);
			std::swap(this_deleter, other_deleter);
		}

		/**
		 * @brief Move constructor
		 */
		unique_ptr(unique_ptr &&other) noexcept
		    : m_underlying_ptr{std::exchange(other.m_underlying_ptr, nullptr)},
		      deleter_type{std::move(static_cast<deleter_type>(other))} {}

		/**
		 * @brief Move assignment operator
		 */
		unique_ptr &operator=(unique_ptr &&other) {
			unique_ptr(std::move(other)).swap(*this);
			return *this;
		}

		/**
		 * @brief Destructor
		 */
		~unique_ptr() {
			deleter_type *deleter = static_cast<deleter_type *>(this);
			(*deleter)(m_underlying_ptr);
		}

		/**
		 * @brief Dererencing operator
		 */
		[[nodiscard]] T &operator*() { return *m_underlying_ptr; }

		/**
		 * @brief Indirection operator
		 */
		[[nodiscard]] T *operator->() { return m_underlying_ptr; }

		/**
		 * @brief Get the underlying raw pointer.
		 */
		[[nodiscard]] T *get() const { return m_underlying_ptr; }

		/**
		 * @brief Index of operator. Provides access to the
		 * managed array.
		 */
		[[nodiscard]] reference operator[](std::size_t index) { return m_underlying_ptr[index]; }

		// Modifiers
		/**
		 * @brief Releases the underlying managed resource
		 */
		[[nodiscard]] T *release() { return std::exchange(m_underlying_ptr, nullptr); }

		/**
		 * @brief Replaces the managed object
		 */
		void reset(T *other) {
			if (m_underlying_ptr != other) {
				(*static_cast<deleter_type *>(this))(m_underlying_ptr);
				m_underlying_ptr = other;
			}
		}

		/**
		 * @brief Implementation of operator bool()
		 */
		explicit operator bool() const { return m_underlying_ptr == nullptr; }

		/**
		 * @brief Implementation of the spaceship operator
		 */
		friend auto operator<=>(const unique_ptr &lhs, const unique_ptr &rhs) { return lhs.get() <=> rhs.get(); }

		/**
		 * @brief Compare a unique_ptr will nullptr
		 */
		friend bool operator==(const unique_ptr &lhs, std::nullptr_t) { return lhs.get() == nullptr; }

		/**
		 * @brief Compare a nullptr will unique_ptr
		 */
		friend bool operator==(std::nullptr_t, const unique_ptr &rhs) { return rhs.get() == nullptr; }

	private:
		pointer m_underlying_ptr;
	};
} // namespace xstd