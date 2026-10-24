#include <cstddef>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace xstd {

	template <typename T> class vector {
		using value_type = T;
		using size_type = std::size_t;
		using pointer = T *;
		using const_pointer = const T *;
		using reference = T &;
		using const_reference = const T &;
		using iterator = pointer;
		using const_iterator = const_pointer;

	private:
		pointer m_data{nullptr};
		size_type m_size{0};
		size_type m_capacity{0};
		constexpr static unsigned short growth_factor{2};

		struct raw_deleter {
			// only frees the memory, doesn't destroy objects
			void operator()(T *ptr) noexcept { operator delete(ptr, std::align_val_t(alignof(value_type))); }
		};

		using raw_ptr = std::unique_ptr<T, raw_deleter>;
		struct init_capacity_tag {
			size_type cap;
		};

		// If an exception happens after this has been called,
		// the destructor will run and deallocate the memory.
		explicit vector(init_capacity_tag cap) : m_data{allocate_helper(cap.cap).release()}, m_capacity{cap.cap} {}

		template <typename U> void push_back_slow_path(U &&value) {
			// allocate more memory
			size_type new_capacity = capacity() ? growth_factor * capacity() : 1;
			size_type offset = size();
			size_type new_size = m_size + 1;
			auto mem = allocate_helper(new_capacity);
			std::construct_at(mem.get() + m_size, std::forward<U>(value));

			try {
				copy_old_storage_to_new(m_data, m_size, mem.get());
			} catch (std::exception &ex) {
				std::destroy_at(mem.get() + m_size);
			}

			pointer ptr_new = mem.release();
			mem.reset(m_data);
			m_data = ptr_new;
			++m_size;
			m_capacity = new_capacity;
		}

		template <typename U> void push_back_fast_path(U &&value) {
			std::construct_at(m_data + m_size, std::forward<U>(value));
			++m_size;
		}

		raw_ptr allocate_helper(size_type new_capacity) {
			auto ptr = operator new(sizeof(value_type) * new_capacity, std::align_val_t(alignof(value_type)));
			return raw_ptr(static_cast<pointer>(ptr));
		}

		// Copies elements from old storage to new
		// If T's copy/move ctor throws, the objects already constructed
		// are destroyed and the exception is propagated to the caller.
		void copy_old_storage_to_new(pointer source_first, size_t num_elements, pointer destination_first) {
			if constexpr (std::is_nothrow_move_constructible_v<T>) {
				std::uninitialized_move(source_first, source_first + num_elements, destination_first);
			} else {
				std::uninitialized_copy(source_first, source_first + num_elements, destination_first);
			}
		}

		template <typename... Args> reference emplace_back_slow_path(Args &&...args) {
			// allocate more memory
			size_type offset = size();
			size_type new_size = m_size + 1;
			size_type new_capacity = capacity() ? growth_factor * capacity() : 1;
			auto mem = allocate_helper(new_capacity);

			std::construct_at(mem.get() + m_size, std::forward<Args>(args)...);
			try {
				copy_old_storage_to_new(m_data, m_size, mem.get());
			} catch (std::exception &ex) {
				std::destroy_at(mem.get() + m_size);
			}

			pointer ptr_new = mem.release();
			mem.reset(m_data);
			m_data = ptr_new;
			++m_size;
			m_capacity = new_capacity;
			return back();
		}

		template <typename... Args> reference emplace_back_fast_path(Args &&...args) {
			std::construct_at(m_data + m_size, std::forward<Args>(args)...);
			++m_size;
			return back();
		}
		bool is_full() { return size() == capacity(); }

	public:
		iterator begin() { return m_data; }
		const_iterator begin() const { return m_data; }
		iterator end() { return begin() + m_size; }
		const_iterator end() const { return begin() + m_size; }

		size_type size() const { return m_size; }
		size_type capacity() const { return m_capacity; }

		vector() noexcept {}

		vector(size_t n, const T &initial_value) : vector(init_capacity_tag(n)) {
			std::uninitialized_fill_n(m_data, n, initial_value);
			m_size = n;
		}

		// Constructs a vector with count default-inserted
		// objects of T. No copies are made.
		explicit vector(size_t n) : vector(init_capacity_tag(n)) {
			std::uninitialized_default_construct(m_data, m_data + n);
			m_size = n;
		}

		vector(std::initializer_list<T> list) : vector(init_capacity_tag(list.size())) {
			std::uninitialized_copy(list.begin(), list.end(), m_data);
			m_size = list.size();
		}

		vector(const_iterator first, const_iterator last) : vector(init_capacity_tag(std::distance(first, last))) {
			if constexpr (std::is_nothrow_move_constructible_v<T>) {
				std::uninitialized_move(first, last, m_data);
			} else {
				std::uninitialized_copy(first, last, m_data);
			}
			m_size = std::distance(first, last);
		}

		vector(const vector &other) : vector(init_capacity_tag(other.capacity())) {
			// Perform a deep-copy of all the Ts
			std::uninitialized_copy(other.m_data, other.m_data + other.m_size, m_data);
			m_size = other.size();
		}

		vector(vector &&other) noexcept
		    : m_data{std::exchange(other.m_data, nullptr)}, m_size{std::exchange(other.m_size, 0)},
		      m_capacity{std::exchange(other.m_capacity, 0)} {}

		void swap(vector &other) noexcept {
			std::swap(this->m_data, other.m_data);
			std::swap(this->m_size, other.m_size);
			std::swap(this->m_capacity, other.m_capacity);
		}

		vector &operator=(const vector &other) {
			vector(other).swap(*this);
			return *this;
		}

		vector &operator=(vector &&other) {
			vector(std::move(other)).swap(*this);
			return *this;
		}

		~vector() {
			std::destroy(begin(), end());
			raw_deleter{}(m_data);
		}

		bool empty() { return m_size == 0; }

		template <typename U> void push_back(U &&value) {
			if (is_full()) {
				push_back_slow_path(std::forward<U>(value));
			} else {
				push_back_fast_path(std::forward<U>(value));
			}
		}

		reference operator[](size_type idx) { return m_data[idx]; }

		const_reference operator[](size_type idx) const { return m_data[idx]; }

		// precondition: !empty()
		reference front() { return (*this)[0]; }
		const_reference front() const { return (*this)[0]; }
		reference back() { return (*this)[m_size - 1]; }
		const_reference back() const { return (*this)[m_size - 1]; }

		const T &at(std::size_t index) const {
			if (index < 0 || index >= m_size)
				throw std::out_of_range("Array index out of bounds!");

			return m_data[index];
		}

		void shrink_to_fit() {

			raw_ptr mem = allocate_helper(m_size);

			// Copy/move the Ts from the old storage to new storage
			if constexpr (std::is_nothrow_move_constructible_v<T>) {
				std::uninitialized_move(m_data, m_data + m_size, mem.get());
			} else {
				std::uninitialized_copy(m_data, m_data + m_size, mem.get());
			}

			// Destroy objects in old storage and deallocate memory
			for (auto p{m_data}; p < m_data + m_size; ++p) {
				std::destroy_at<T>(p);
			}
			// Deallocate memory
			pointer new_ptr = mem.release();
			mem.reset(m_data);

			// Reassign internal buffer pointer and set size and capacity
			m_data = new_ptr;
			m_capacity = m_size;
		}

		void pop_back() {
			T *ptr_to_last = m_data + m_size - 1;
			std::destroy_at(ptr_to_last);
			--m_size;
		}

		bool operator==(const vector &other) const {
			return size() == other.size() && std::equal(begin(), end(), other.begin());
		}

		void reserve(size_type new_capacity) {
			if (new_capacity <= capacity())
				return;

			raw_ptr mem = allocate_helper(new_capacity);
			copy_old_storage_to_new(m_data, m_size, mem.get()); // can throw

			std::destroy(m_data, m_data + m_size);
			pointer new_ptr = mem.release();
			mem.reset(m_data);
			m_data = new_ptr;
			m_capacity = new_capacity;
		}

		void resize(size_type new_size) {
			size_type current_size = m_size;
			if (new_size == current_size)
				return;

			if (new_size < current_size) {
				// Reduce the container to count elements
				std::destroy(m_data + new_size, m_data + m_size);
			}

			if (new_size > current_size) {
				reserve(new_size);

				// Default construct elements at indicates
				// [current_size,...,new_size-1]
				std::uninitialized_value_construct(begin() + current_size, begin() + new_size);
			}
			m_size = new_size;
		}

		template <typename... Args> reference emplace_back(Args &&...args) {
			if (is_full())
				return emplace_back_slow_path(std::forward<Args>(args)...);
			else
				return emplace_back_fast_path(std::forward<Args>(args)...);
		}

		template <typename It> iterator insert(const_iterator pos, It first, It last) {
			auto pos_ = const_cast<iterator>(pos);
			auto first_ = first;
			auto last_ = last;

			if (first != last) {
				size_type offset = std::distance(begin(), pos_);
				size_type n = std::distance(first, last);
				size_type num_elems_to_shift = std::distance(pos_, end());
				size_type remaining_capacity = capacity() - size();

				xstd::vector<T> temp;
				// handle self-referential insertion
				if ((first_ >= begin() && first_ < end()) && last_ > begin() && last_ <= end()) {
					for (auto it{first_}; it != last_; ++it)
						temp.push_back(*it);

					first_ = temp.begin();
					last_ = temp.end();
				}

				if (n > remaining_capacity) {
					size_type excess_capacity_reqd = std::max(static_cast<int>(n - remaining_capacity), 0);
					reserve(capacity() + excess_capacity_reqd);
					// The iterator pos is invalidated. Update it.
					pos_ = std::next(begin(), offset);
				}

				// objects to displace (move or copy) from the
				// [begin, end()] sequence into raw uninitialized
				// memory
				if (n < num_elems_to_shift) {
					if constexpr (std::is_nothrow_move_constructible_v<T>) {
						std::uninitialized_move(end() - n, end(), end());
					} else {
						std::uninitialized_copy(end() - n, end(), end());
					}

				} else {
					size_type num_tail = std::max(static_cast<int>(n - num_elems_to_shift), 0);
					if constexpr (std::is_nothrow_move_constructible_v<T>) {
						std::uninitialized_move(pos_, end(), end() + num_tail);
					} else {
						std::uninitialized_copy(pos_, end(), end() + num_tail);
					}
				}

				// objects to displace (copy or move) from [pos,end()]
				// to the end of the container
				if (n < num_elems_to_shift) {
					if constexpr (std::is_nothrow_move_constructible_v<T>) {
						std::move_backward(pos_, end() - n, end());
					} else {
						std::copy_backward(pos_, end() - n, end());
					}
				}

				// objects from [first,last) to insert into raw
				// uninitialized memory
				const size_type num_tail = std::max(static_cast<int>(n - num_elems_to_shift), 0);
				if (n >= num_elems_to_shift) {
					if constexpr (std::is_nothrow_move_constructible_v<T>) {
						std::uninitialized_move(last_ - num_tail, last_, end());
					} else {
						std::uninitialized_copy(last_ - num_tail, last_, end());
					}
				}

				// objects to copy from [first,last) to pos
				if (n < num_elems_to_shift) {
					std::copy(first_, last_, pos_);
				} else {
					std::copy(first_, first_ + n - num_tail, pos_);
				}

				m_size += n;
			}
			return pos_;
		}
	};
} // namespace xstd