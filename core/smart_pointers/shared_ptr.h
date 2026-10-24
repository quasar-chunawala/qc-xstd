#include <atomic>
#include <cassert>
#include <memory>
#include <utility>

namespace xstd {

	template <typename T> class shared_ptr_base {
	public:
		using value_type = T;
		using pointer = T *;
		using const_pointer = const T *;
		using reference = T &;
		using const_reference = const T &;

	protected:
		struct destroy_wrapper // RAII class
		{
			template <typename Deleter>
			explicit destroy_wrapper(Deleter &&deleter)
			    : m_destroyer_ptr{std::make_unique<destroyer<Deleter>>(std::forward<Deleter>(deleter))} {}
			// destroy_wrapper is a wrapper over a
			// unique_ptr<destroyer_base>. It is intended to be
			// move-constructible ONLY.
			destroy_wrapper(destroy_wrapper &&other) noexcept
			    : m_destroyer_ptr{std::exchange(other.m_destroyer_ptr, nullptr)} {}

			void operator()(pointer ptr) {
				(*m_destroyer_ptr)(ptr); // Virtual polymorphism
			}

			struct destroyer_base {
				virtual void operator()(pointer ptr) = 0;
				virtual ~destroyer_base() = default;
			};

			template <typename Deleter> struct destroyer : public destroyer_base {
				explicit destroyer(Deleter deleter) : destroyer_base(), m_deleter{deleter} {}

				void operator()(pointer ptr) override { m_deleter(ptr); }

				Deleter m_deleter;
			};

			~destroy_wrapper() {}
			std::unique_ptr<destroyer_base> m_destroyer_ptr;
		};

		struct control_block_base {
			std::atomic<unsigned long long> m_ref_count;
			destroy_wrapper m_destroy_wrapper;

			/**
			 * @brief Default constructor
			 */
			control_block_base() : control_block_base(0u, destroy_wrapper(std::default_delete<T>())) {}

			explicit control_block_base(destroy_wrapper &&wrapper) : control_block_base(0u, std::move(wrapper)) {}

			explicit control_block_base(unsigned long long ref_count, destroy_wrapper &&wrapper)
			    : m_destroy_wrapper{std::move(wrapper)} {
				m_ref_count.store(ref_count);
			}

			/**
			 * @brief helper function to increment the object reference
			 * count
			 */
			void increment() { m_ref_count.fetch_add(1u); }

			/**
			 * @brief helper function to decrement the object reference
			 * count
			 */
			auto decrement() {
				auto result = m_ref_count.fetch_sub(1u);
				return result;
			}

			// There is no use-case for copying control blocks of a
			// shared_ptr<T> instance. For safety, I delete these
			// functions.
			control_block_base(const control_block_base &) = delete;
			control_block_base &operator=(const control_block_base &) = delete;
			control_block_base(control_block_base &&) = delete;
			control_block_base &operator=(control_block_base &&) = delete;

			/**
			 * @brief Return the object reference count
			 */
			[[nodiscard]] unsigned long long use_count() const noexcept { return m_ref_count.load(); }

			virtual void release_shared(T *) = 0;
			virtual ~control_block_base() {}
		};

		struct control_block : public control_block_base {
			// pointer m_object_ptr;

			control_block() : control_block_base() {}

			explicit control_block(unsigned long long ref_count, destroy_wrapper &&wrapper)
			    : control_block_base(ref_count, std::move(wrapper)) {}

			explicit control_block(destroy_wrapper &&wrapper) : control_block_base(std::move(wrapper)) {}

			void release_shared(T *raw_underlying_ptr) override {
				if (--this->m_ref_count == 0) {
					control_block_base::m_destroy_wrapper(raw_underlying_ptr);
					delete this;
				}
			}

			~control_block() {}
		};

		/**
		 * @brief This is a specialized version of %control_block_base,
		 * where the managed object resides within the control block
		 * itself.
		 */
		struct control_block_with_storage : public control_block_base {
			T m_object;

			/**
			 * @brief Instantiates the managed object by perfectly
			 * forwarding the constructor args.
			 */
			template <typename... Args>
			explicit control_block_with_storage(Args &&...args)
			    : control_block_base{1u, destroy_wrapper(std::default_delete<T>())},
			      m_object(std::forward<Args>(args)...) {}

			/**
			 * @brief C'tor that accepts a destroyer object and perfectly
			 * forwards the constructor args
			 */
			template <typename... Args>
			explicit control_block_with_storage(destroy_wrapper &&wrapper, Args &&...args)
			    : control_block_base{1u, std::move(wrapper)}, m_object(std::forward<Args>(args)...) {}

			void release_shared(T *) override {
				if (--control_block_base::m_ref_count == 0) {
					delete this;
				}
			}

			T *get() { return &m_object; }
		};

	public:
		/**
		 * @brief Default constructor - an empty shared_ptr
		 */
		shared_ptr_base() : shared_ptr_base(nullptr) {}

		/**
		 * @brief shared_ptr
		 */
		shared_ptr_base(std::nullptr_t) : m_raw_underlying_ptr{nullptr}, m_control_block_ptr{new control_block()} {}

		/**
		 * @brief Constructor that takes a raw pointer. Takes
		 * ownership of the pointee.
		 */
		explicit shared_ptr_base(pointer ptr) : m_raw_underlying_ptr{ptr}, m_control_block_ptr{nullptr} {
			// The child-classes will allocate the control_block
		}

		/**
		 * @brief Constructor that takes a raw pointer and a custom
		 * deleter. Takes ownership of the pointee.
		 */
		explicit shared_ptr_base(T *ptr, destroy_wrapper &&destroyer)
		    : m_raw_underlying_ptr{ptr}, m_control_block_ptr{nullptr} {
			try {
				if (ptr) {
					m_control_block_ptr = new control_block(1u, std::move(destroyer));
				} else {
					m_control_block_ptr = new control_block(std::move(destroyer));
				}

			} catch (std::exception &ex) {
				destroyer(ptr);
				throw ex;
			}
		}

		explicit shared_ptr_base(T *ptr, control_block_base *cb) : m_raw_underlying_ptr{ptr}, m_control_block_ptr{cb} {}

		/**
		 * @brief Copy constructor. Models shared co-ownership of the
		 * resource semantics.
		 */
		shared_ptr_base(const shared_ptr_base &other)
		    : m_raw_underlying_ptr{other.m_raw_underlying_ptr}, m_control_block_ptr{other.m_control_block_ptr} {
			if (m_control_block_ptr)
				m_control_block_ptr->increment(); // Atomic pre-increment
		}

		/**
		 * @brief Move constructor. Represents transfer of ownership.
		 */
		shared_ptr_base(shared_ptr_base &&other) noexcept
		    : m_raw_underlying_ptr{std::exchange(other.m_raw_underlying_ptr, nullptr)},
		      m_control_block_ptr{std::exchange(other.m_control_block_ptr, nullptr)} {}

		/**
		 * @brief Swaps two shared-pointer objects member-by-member.
		 */
		void swap(shared_ptr_base &other) noexcept {
			std::swap(m_raw_underlying_ptr, other.m_raw_underlying_ptr);
			std::swap(m_control_block_ptr, other.m_control_block_ptr);
		}

		friend void swap(shared_ptr_base &lhs, shared_ptr_base &rhs) noexcept { lhs.swap(rhs); }

		/**
		 * @brief Copy assignment operator. Release the currently held
		 * resource and become a shared co-owner of the resource specified
		 * by user-supplied argument @a other.
		 */
		shared_ptr_base &operator=(const shared_ptr_base &other) {
			shared_ptr_base{other}.swap(*this);
			return *this;
		}

		/**
		 * @brief Move assignment operator. Release the currently held
		 * resource. Transfer ownership of the resource.
		 */
		shared_ptr_base &operator=(shared_ptr_base &&other) {
			shared_ptr_base{std::move(other)}.swap(*this);
			return *this;
		}

		/**
		 * @brief Destructor
		 */
		~shared_ptr_base() {
			if (m_raw_underlying_ptr) {
				m_control_block_ptr->release_shared(m_raw_underlying_ptr);
				m_raw_underlying_ptr = nullptr;
			}
		}

		// Pointer-like functions
		/**
		 * @brief Returns the raw underlying pointer.
		 */
		[[nodiscard]] pointer get() { return m_raw_underlying_ptr; }

		/**
		 * @brief Returns a %pointer-to-const
		 */
		[[nodiscard]] const_pointer get() const { return m_raw_underlying_ptr; }

		/**
		 * @brief Returns a reference to the managed object
		 */
		[[nodiscard]] reference operator*() noexcept { return *m_raw_underlying_ptr; }

		/**
		 * @brief Returns a reference-to-const.
		 */
		[[nodiscard]] const_reference operator*() const noexcept { return *m_raw_underlying_ptr; }

		/**
		 * @brief Implementation of the indirection operator
		 */
		[[nodiscard]] pointer operator->() noexcept { return m_raw_underlying_ptr; }

		/**
		 * @brief Implementation of the indirection operator
		 */
		[[nodiscard]] const_pointer operator->() const noexcept { return m_raw_underlying_ptr; }

		/**
		 * @brief Spaceship operator.
		 */
		friend auto operator<=>(const shared_ptr_base &lhs, const shared_ptr_base &rhs) {
			return lhs.m_raw_underlying_ptr <=> rhs.m_raw_underlying_ptr;
		}

		/**
		 * @brief Equality comparison operator
		 */
		[[nodiscard]] bool operator==(const shared_ptr_base &other) const noexcept {
			return (m_raw_underlying_ptr == other.m_raw_underlying_ptr);
		}

		[[nodiscard]] bool operator==(std::nullptr_t) const noexcept { return m_raw_underlying_ptr == nullptr; }

		/**
		 * @brief Returns the reference count of the managed object
		 */
		[[nodiscard]] unsigned long long use_count() const noexcept {
			if (m_control_block_ptr)
				return m_control_block_ptr->use_count();
			else
				return 0;
		}

		template <typename... Args> shared_ptr_base(Args &&...args) {
			/* Perform a single heap memory allocation */
			control_block_with_storage *cb = new control_block_with_storage(destroy_wrapper(std::default_delete<T>()),
			                                                                (std::forward<Args>(args))...);
			m_raw_underlying_ptr = cb->get();
			m_control_block_ptr = cb;
		}

		void reset_base(T *ptr, destroy_wrapper &&wrapper) {
			if (m_raw_underlying_ptr != ptr) {
				if (--m_control_block_ptr->m_ref_count == 0) {
					m_control_block_ptr->m_destroy_wrapper(m_raw_underlying_ptr);
					m_control_block_ptr->m_ref_count.store(1u);
				} else {
					// Multiple ownership
					m_control_block_ptr = new control_block(1u, std::move(wrapper));
				}
				shared_ptr_base<T>::m_raw_underlying_ptr = ptr;
			}
		}

	protected:
		T *m_raw_underlying_ptr;
		control_block_base *m_control_block_ptr;
	};

	template <typename T> class shared_ptr : public shared_ptr_base<T> {
	public:
		using control_block = shared_ptr_base<T>::control_block;
		using destroy_wrapper = typename shared_ptr_base<T>::destroy_wrapper;
		shared_ptr() : shared_ptr_base<T>() {}

		shared_ptr(std::nullptr_t) : shared_ptr_base<T>(nullptr) {}

		explicit shared_ptr(T *ptr) : shared_ptr_base<T>(ptr) {

			try {
				shared_ptr_base<T>::m_control_block_ptr =
				    new control_block(1u, destroy_wrapper(std::default_delete<T>()));
			} catch (std::exception &ex) {
				delete ptr;
				// We may want to log the exception here
				throw ex;
			}
		}

		template <typename Deleter>
		explicit shared_ptr(T *ptr, Deleter deleter) : shared_ptr_base<T>(ptr, destroy_wrapper(deleter)) {}

		template <typename... Args>
		explicit shared_ptr(Args... args) : shared_ptr_base<T>(std::forward<Args>(args)...) {}

		/**
		 * @brief Replaces the managed object.
		 */
		void reset(T *ptr) { shared_ptr_base<T>::reset_base(ptr, destroy_wrapper(std::default_delete<T>())); }
	};

	template <typename T> class shared_ptr<T[]> : public shared_ptr_base<T> {
	public:
		using control_block = shared_ptr_base<T>::control_block;
		using destroy_wrapper = typename shared_ptr_base<T>::destroy_wrapper;

		shared_ptr() : shared_ptr(nullptr) {}
		shared_ptr(std::nullptr_t) : shared_ptr_base<T>{nullptr, destroy_wrapper(std::default_delete<T[]>())} {}

		explicit shared_ptr(T *ptr) : shared_ptr_base<T>(ptr) {
			try {
				shared_ptr_base<T>::m_control_block_ptr =
				    new control_block(1u, destroy_wrapper(std::default_delete<T[]>()));
			} catch (std::exception &ex) {
				delete[] ptr;
				// We may want to log the exception here
				throw ex;
			}
		}

		template <typename Deleter = std::default_delete<T[]>>
		explicit shared_ptr(T *ptr, Deleter deleter) : shared_ptr_base<T>(ptr, destroy_wrapper(deleter)) {}

		template <typename... Args>
		explicit shared_ptr(Args... args) : shared_ptr_base<T[]>(std::forward<Args>(args)...) {}

		/**
		 * @brief Replaces the managed object.
		 */
		void reset(T *ptr) { shared_ptr_base<T>::reset_base(ptr, destroy_wrapper(std::default_delete<T[]>())); }

		T &operator[](int n) { return shared_ptr_base<T>::m_raw_underlying_ptr[n]; }
	};

	/**
	 * @brief %make_shared is a utility function that accepts constructor
	 * args and perfoms a single heap memory allocation for both the
	 * managed resource and the control block.
	 */
	template <typename T, typename... Args>
	shared_ptr<T> // Single-object version
	make_shared(Args &&...args) {
		return shared_ptr<T>(std::forward<Args>(args)...);
	}

	// TODO: Implement the array-version of make_shared<T[]>() available
	// since C++20

} // namespace xstd