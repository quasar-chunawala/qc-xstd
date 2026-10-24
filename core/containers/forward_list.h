#include <algorithm>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <utility>

namespace xstd {
	template <typename T> class forward_list {
	public:
		using value_type = T;
		using size_type = std::size_t;
		using pointer = T *;
		using const_pointer = const T *;
		using reference = T &;
		using const_reference = const T &;
		using difference_type = std::ptrdiff_t;

	private:
		struct ListNode {
			value_type m_value;
			ListNode *next{nullptr};

			ListNode(const_reference value) : m_value{value} {}

			ListNode(T &&value) : m_value{std::move(value)} {}
		};

		ListNode *m_head{nullptr};
		size_type m_size{0};

	public:
		template <typename U> class Iterator {
		public:
			using value_type = forward_list::value_type;
			using pointer = forward_list::pointer;
			using reference = forward_list::reference;
			using difference_type = forward_list::difference_type;
			friend class forward_list<T>;

		private:
			ListNode *m_current_node_ptr;

		public:
			Iterator() = default;
			Iterator(ListNode *ptr) : m_current_node_ptr{ptr} {}

			// Pre-increment
			Iterator &operator++() {
				m_current_node_ptr = m_current_node_ptr->next;
				return *this;
			}

			// Post-increment
			Iterator operator++(int) {
				auto temp = *this;
				m_current_node_ptr = m_current_node_ptr->next;
				return temp;
			}

			bool operator==(const Iterator &other) const { return (m_current_node_ptr == other.m_current_node_ptr); }

			bool operator!=(const Iterator &other) const { return !(*this == other); }

			// Pointer-like operations
			U &operator*() { return m_current_node_ptr->m_value; }

			const U &operator*() const { return m_current_node_ptr->m_value; }

			U *operator->() { return m_current_node_ptr; }

			const U *operator->() const { return m_current_node_ptr; }
		};

		using iterator = Iterator<T>;
		using const_iterator = Iterator<const T>;

		size_type size() const { return m_size; }

		bool empty() const { return (!m_head); }

		iterator begin() { return iterator(m_head); }

		const_iterator begin() const { return const_iterator(m_head); }

		const_iterator cbegin() const { return const_iterator(m_head); }

		iterator end() { return iterator(nullptr); }

		const_iterator end() const { return const_iterator(nullptr); }

		const_iterator cend() const { return end(); }

		/* Destroy the containers content */
		void clear() noexcept {
			for (auto p{m_head}; p != nullptr;) {
				auto q = p->next;
				delete p;
				p = q;
			}
			m_size = 0;
		}

		~forward_list() { clear(); }

		/* Constructors */
		forward_list() = default;

		template <std::input_iterator It> forward_list(It b, It e) {
			if (b == e)
				return;
			ListNode *m_curr{nullptr};

			try {
				for (auto it{b}; it != e; ++it) {
					if (empty()) {
						m_head = new ListNode(*it);
						++m_size;
						m_curr = m_head;
					} else {
						m_curr->next = new ListNode(*it);
						++m_size;
						m_curr = m_curr->next;
					}
				}
			} catch (...) {
				clear();
				throw;
			}
		}

		forward_list(const forward_list &other) : forward_list(other.begin(), other.end()) {}

		forward_list(std::initializer_list<T> other) : forward_list(other.begin(), other.end()) {}

		forward_list(forward_list &&other)
		    : m_head{std::exchange(other.m_head, nullptr)}, m_size{std::exchange(other.m_size, 0)} {}

		void swap(forward_list &other) {
			using std::swap;
			swap(m_head, other.m_head);
			swap(m_size, other.m_size);
		}

		forward_list &operator=(const forward_list &other) {
			forward_list(other).swap(*this);
			return *this;
		}

		forward_list &operator=(forward_list &&other) { forward_list(std::move(other)).swap(*this); }

		iterator before_begin() noexcept { return (m_head - 1); }

		const_iterator cbefore_begin() noexcept { return (m_head - 1); }

		/* Modifiers */

		// insert_after - Inserts elements after the specified position
		// in the container. No iterators or references are invalidated.
		// If pos is not in the range [before_begin(), end()) then
		// the behavior is UB.
	private:
		void insert_after_helper(auto pos, ListNode *newNode) {
			if (std::next(pos) == begin()) {
				newNode->next = m_head;
				m_head = newNode;
			} else {
				newNode->next = pos->next;
			}
			pos->next = newNode;
			++m_size;
		}

	public:
		iterator insert_after(const_iterator pos, const T &value) {
			ListNode *node = new ListNode(value);
			insert_after_helper(pos, node);
			return node;
		}

		iterator insert_after(const_iterator pos, T &&value) {
			ListNode *node = new ListNode(std::move(value));
			insert_after_helper(pos, node);
			return node;
		}

		// emplace_after - Inserts a new element into a position after the
		// specified position in the container. The element is
		// constructed in-place.
		template <typename... Args> iterator emplace_after(const_iterator pos, Args &&...args) {
			ListNode *node = new ListNode(T(std::forward<Args>(args)...));
			insert_after_helper(pos, node);
			return node;
		}

		// erase_after - removes the element following pos
		// It returns iterator to the element following pos
		// or end(), if no such element exists
		iterator erase_after(const_iterator pos) {
			if (pos == end() || std::next(pos) == end())
				return end();

			auto p = pos->next;
			auto q = p->next;
			pos->next = q;
			delete p;
			--m_size;
			return q;
		}

		template <typename U> iterator push_front(U &&value) {
			return insert_after(before_begin(), std::forward<U>(value));
		}

		template <typename... Args> iterator emplace_front(Args &&...args) {
			return emplace_after(before_begin(), std::forward<Args>(args)...);
		}

		// pop_front - Removes the first element in the container
		void pop_front() {
			if (m_head) {
				auto p = m_head;
				m_head = m_head->next;
				delete p;
				--m_size;
			}
		}
		// resize() - Resizes the container to contain count elements
		// - if the count is equal to the current size, does nothing.
		// - if the current size > count, then the container is reduced to its
		//   first count elements.
		// - if the current size is less than count, then additional default-inserted
		//   elements/copies of value are appended.
		void resize(size_type count) {
			if (count == m_size)
				return;

			if (count < m_size) {
				if (count == 0) {
					clear();
					return;
				}

				int running_count{0};
				for (auto p{m_head}; p != nullptr;) {
					auto q = p->next;
					if (running_count == count) {
						delete p;
					}
					++running_count;
					p = q;
				}
			}

			if (count > m_size) {
				if (empty())
					push_front(T());

				auto p{m_head};
				int size{m_size};

				for (auto running_count{1}; running_count <= count; ++running_count) {
					if (running_count > size)
						insert_after(p, T{});
					p = p->next;
				}
			}
		}
	};
} // namespace xstd
