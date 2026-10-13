#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <initializer_list>
#include <iterator>

namespace dev {

	struct string_long {
		char *m_buffer_ptr; // 8 bytes
		size_t m_size;      // 8 bytes
		size_t m_capacity;  // 8 bytes
	};

	struct string_short {
		static constexpr size_t capacity{23};
		char m_buffer[capacity]; // 23 bytes
		unsigned char m_size;    // 1 byte
	};

	class string {
		using iterator = char *;
		using const_iterator = const char *;

	private:
		union {
			string_short s;
			string_long l;
		} m_data;

		static constexpr size_t string_short_mask = 0x01;
		static constexpr size_t growth_factor = 2;

		// --- SSO flag helpers ---
		bool is_short_string();
		bool is_short_string() const;
		bool is_long_string();
		bool is_long_string() const;

		// --- size helpers ---
		size_t get_short_size();
		size_t get_short_size() const;
		void set_short_size(size_t size);
		size_t get_long_size();
		size_t get_long_size() const;
		void set_long_size(size_t size);
		void set_size(size_t size);
		void set_sso_flag();
		void turn_off_sso_flag();

		// --- capacity helpers ---
		size_t get_long_capacity();
		size_t get_short_capacity();
		void set_long_capacity(size_t capacity);

		// --- allocation helpers ---
		char *allocate_helper(size_t new_capacity);
		void deallocate_helper(char *buffer_ptr);
		void construct_slow_path(const char *chars_array, size_t len);
		void construct_fast_path(const char *chars_array, size_t len);

		bool is_full();

	public:
		// --- constructors ---
		string();
		string(const char *chars_array);
		string(const string &other);
		string(string &&other);

		// construct from an arbitrary range [first, last)
		template <typename InputIt>
		string(InputIt first, InputIt last) : string() {
			for (auto it{first}; it != last; ++it)
				push_back(*it);
		}

		// construct from an initializer list
		string(std::initializer_list<char> list)
		    : string{list.begin(), list.end()} {}

		// construct count copies of character ch
		string(size_t count, char ch);

		// --- destructor ---
		~string();

		// --- swap ---
		friend void swap(string &lhs, string &rhs);
		void swap(string &other);

		// --- assignment ---
		string &operator=(const string &other);
		string &operator=(string &&other);

		// --- capacity ---
		size_t size();
		size_t size() const;
		size_t capacity();
		bool empty();
		void reserve(size_t new_capacity);

		// --- element access ---
		char *data();
		const char *data() const;
		char &operator[](size_t pos);
		const char &operator[](size_t pos) const;
		char &at(size_t pos);
		char &front();
		char &back();

		// --- iterators ---
		char *begin();
		const char *begin() const;
		char *end();
		const char *end() const;

		// --- modifiers ---
		void push_back(char ch);
		void pop_back();

		// insert range [first, last) before pos
		template <typename InputIt>
		void insert(const_iterator pos, InputIt first, InputIt last) {
			auto pos_ = const_cast<iterator>(pos);
			auto first_ = first;
			auto last_ = last;
			if (first < last) {
				size_t offset = std::distance(begin(), pos_);
				size_t n = std::distance(first, last);
				size_t num_elems_to_shift = std::distance(pos_, end());
				size_t remaining_capacity = capacity() - size();

				// handle self-referential insertion
				string temp;
				if (first_ >= begin() && first_ < end() &&
				    last_ > begin() && last_ <= end()) {
					for (auto it{first_}; it != last_; ++it)
						temp.push_back(*it);
					first_ = temp.begin();
					last_ = temp.end();
				}

				if (n > remaining_capacity) {
					size_t excess_capacity_reqd = std::max(
					    static_cast<int>(n - remaining_capacity), 0);
					reserve(capacity() + excess_capacity_reqd);
					pos_ = std::next(begin(), offset);
				}

				if (n < num_elems_to_shift) {
					std::copy(end() - n, end(), end());
					std::copy_backward(pos_, end() - n, end());
				} else {
					std::copy(pos_, end(), pos_ + n);
				}

				std::copy(first_, last_, pos_);
				set_size(size() + n);
				char *buffer_ptr = data();
				buffer_ptr[size()] = '\0';
			}
		}

		iterator erase(const_iterator first, const_iterator last);
	};

} // namespace dev

namespace dev {

	// ============================================================
	// SSO flag helpers
	// ============================================================

	bool string::is_short_string() {
		return m_data.s.m_size & string_short_mask;
	}
	bool string::is_short_string() const {
		return m_data.s.m_size & string_short_mask;
	}
	bool string::is_long_string() {
		return !is_short_string();
	}
	bool string::is_long_string() const {
		return !is_short_string();
	}

	// ============================================================
	// Size helpers
	// ============================================================

	size_t string::get_short_size() {
		return m_data.s.m_size >> 1;
	}
	size_t string::get_short_size() const {
		return m_data.s.m_size >> 1;
	}
	void string::set_short_size(size_t sz) {
		m_data.s.m_size = (sz << 1) | string_short_mask;
	}
	size_t string::get_long_size() {
		return m_data.l.m_size;
	}
	size_t string::get_long_size() const {
		return m_data.l.m_size;
	}
	void string::set_long_size(size_t sz) {
		m_data.l.m_size = sz;
	}

	void string::set_size(size_t sz) {
		if (is_short_string())
			set_short_size(sz);
		else
			set_long_size(sz);
	}

	void string::set_sso_flag() {
		m_data.s.m_size |= string_short_mask;
	}
	void string::turn_off_sso_flag() {
		m_data.l.m_capacity &= size_t(~string_short_mask);
	}

	// ============================================================
	// Capacity helpers
	// ============================================================

	size_t string::get_long_capacity() {
		return m_data.l.m_capacity;
	}
	size_t string::get_short_capacity() {
		return string_short::capacity;
	}
	void string::set_long_capacity(size_t cap) {
		assert(!(cap & 0x01));
		m_data.l.m_capacity = cap;
	}

	// ============================================================
	// Allocation helpers
	// ============================================================

	char *string::allocate_helper(size_t new_capacity) {
		assert(!(new_capacity & 0x01));
		return static_cast<char *>(operator new(new_capacity));
	}

	void string::deallocate_helper(char *buffer_ptr) {
		operator delete(buffer_ptr);
	}

	void string::construct_slow_path(const char *chars_array, size_t len) {
		size_t bytes_to_alloc = ((len + 1) & 0x01) ? len + 2 : len + 1;
		m_data.l.m_buffer_ptr = allocate_helper(bytes_to_alloc);
		for (auto i{0}; i < len; ++i)
			m_data.l.m_buffer_ptr[i] = chars_array[i];
		m_data.l.m_buffer_ptr[len] = '\0';
		set_long_size(len);
		turn_off_sso_flag();
		set_long_capacity(bytes_to_alloc);
	}

	void string::construct_fast_path(const char *chars_array, size_t len) {
		for (auto i{0}; i < len; ++i)
			m_data.s.m_buffer[i] = chars_array[i];
		m_data.s.m_buffer[len] = '\0';
		set_short_size(len);
		set_sso_flag();
	}

	bool string::is_full() {
		return size() == capacity();
	}

	// ============================================================
	// Constructors
	// ============================================================

	string::string() {
		m_data.s.m_buffer[0] = '\0';
		set_short_size(0);
		set_sso_flag();
	}

	string::string(const char *chars_array) : string() {
		size_t len = strlen(chars_array);
		if (len > 0 && len < string_short::capacity)
			construct_fast_path(chars_array, len);
		else
			construct_slow_path(chars_array, len);
	}

	string::string(const string &other) {
		if (other.is_short_string()) {
			size_t len = other.size();
			for (auto i{0}; i < len; ++i)
				m_data.s.m_buffer[i] = other[i];
			set_short_size(len);
			m_data.s.m_buffer[len] = '\0';
		} else {
			m_data.l.m_buffer_ptr =
			    allocate_helper(other.m_data.l.m_capacity);
			size_t len = other.get_long_size();
			for (size_t i{0}; i < len; ++i)
				m_data.l.m_buffer_ptr[i] = other[i];
			m_data.l.m_size = len;
			m_data.l.m_capacity = other.m_data.l.m_capacity;
			m_data.l.m_buffer_ptr[len] = '\0';
		}
	}

	string::string(string &&other) {
		if (other.is_short_string()) {
			size_t len = other.size();
			for (size_t i{0}; i < len; ++i)
				m_data.s.m_buffer[i] = other[i];
			set_short_size(len);
			m_data.s.m_buffer[len] = '\0';
			other.m_data.s.m_buffer[0] = '\0';
			other.set_short_size(0);
		} else {
			m_data.l.m_buffer_ptr =
			    std::exchange(other.m_data.l.m_buffer_ptr, nullptr);
			m_data.l.m_size = std::exchange(other.m_data.l.m_size, 0);
			m_data.l.m_capacity =
			    std::exchange(other.m_data.l.m_capacity, 0);
			m_data.l.m_buffer_ptr[other.m_data.l.m_size] =
			    '\0'; // size already 0 after exchange
			// Re-null-terminate using the captured length
			size_t len = m_data.l.m_size;
			m_data.l.m_buffer_ptr[len] = '\0';
		}
	}

	string::string(size_t count, char ch) : string() {
		for (size_t i{0}; i < count; ++i)
			push_back(ch);
	}

	// ============================================================
	// Destructor
	// ============================================================

	string::~string() {
		if (is_long_string()) {
			deallocate_helper(m_data.l.m_buffer_ptr);
			m_data.l.m_buffer_ptr = nullptr;
			m_data.l.m_size = 0;
		}
	}

	// ============================================================
	// Swap
	// ============================================================

	void swap(string &lhs, string &rhs) {
		auto temp = std::move(lhs.m_data);
		lhs.m_data = std::move(rhs.m_data);
		rhs.m_data = std::move(temp);
	}

	void string::swap(string &other) {
		using std::swap;
		swap(*this, other);
	}

	// ============================================================
	// Assignment
	// ============================================================

	string &string::operator=(const string &other) {
		if (this == &other)
			return *this;
		string(other).swap(*this);
		return *this;
	}

	string &string::operator=(string &&other) {
		string(std::move(other)).swap(*this);
		return *this;
	}

	// ============================================================
	// Capacity
	// ============================================================

	size_t string::size() {
		return is_short_string() ? get_short_size() : get_long_size();
	}

	size_t string::size() const {
		return is_short_string() ? get_short_size() : get_long_size();
	}

	size_t string::capacity() {
		return is_short_string() ? get_short_capacity() - 1
		                         : get_long_capacity() - 1;
	}

	bool string::empty() {
		return size() == 0;
	}

	void string::reserve(size_t new_capacity) {
		size_t current_capacity = capacity();
		size_t current_size = size();

		if (new_capacity <= current_capacity ||
		    new_capacity < string_short::capacity)
			return;

		++new_capacity;
		new_capacity =
		    !(new_capacity & 0x01) ? new_capacity : new_capacity + 1;

		char *mem = allocate_helper(new_capacity);
		char *buffer_ptr = data();
		for (size_t i{0}; i < current_size; ++i)
			mem[i] = buffer_ptr[i];

		if (is_long_string())
			deallocate_helper(buffer_ptr);

		m_data.l.m_buffer_ptr = mem;
		m_data.l.m_capacity = new_capacity;
		set_long_size(current_size);
		turn_off_sso_flag();
	}

	// ============================================================
	// Element access
	// ============================================================

	char *string::data() {
		return is_short_string() ? &m_data.s.m_buffer[0]
		                         : m_data.l.m_buffer_ptr;
	}
	const char *string::data() const {
		return is_short_string() ? &m_data.s.m_buffer[0]
		                         : m_data.l.m_buffer_ptr;
	}

	char &string::operator[](size_t pos) {
		return data()[pos];
	}
	const char &string::operator[](size_t pos) const {
		return data()[pos];
	}

	char &string::at(size_t pos) {
		if (pos < 0 || pos > size())
			throw std::out_of_range("Index out of bounds!");
		return data()[pos];
	}

	char &string::front() {
		return data()[0];
	}
	char &string::back() {
		return data()[size() - 1];
	}

	// ============================================================
	// Iterators
	// ============================================================

	char *string::begin() {
		return data();
	}
	const char *string::begin() const {
		return data();
	}
	char *string::end() {
		return data() + size();
	}
	const char *string::end() const {
		return data() + size();
	}

	// ============================================================
	// Modifiers
	// ============================================================

	void string::push_back(char ch) {
		size_t current_size = size();
		if (is_full()) {
			size_t new_capacity = capacity() * growth_factor;
			reserve(new_capacity);
		}
		char *buffer_ptr = data();
		buffer_ptr[current_size] = ch;
		buffer_ptr[current_size + 1] = '\0';
		set_size(current_size + 1);
	}

	void string::pop_back() {
		size_t new_size = size() - 1;
		set_size(new_size);
		data()[new_size] = '\0';
	}

	string::iterator string::erase(const_iterator first,
	                               const_iterator last) {
		iterator first_ = const_cast<iterator>(first);
		iterator last_ = const_cast<iterator>(last);
		size_t offset = std::distance(begin(), first_);

		size_t num_chars_to_shift_left = std::distance(last_, end());
		last_ = std::min(last_, end());
		string chars_to_remove(first_, last_);
		size_t num_chars_erased = std::distance(first_, last_);

		char *buffer_ptr = begin();
		for (size_t i{0}; i < num_chars_to_shift_left; ++i)
			buffer_ptr[offset + i] =
			    buffer_ptr[offset + i + num_chars_erased];

		set_size(size() - num_chars_erased);
		buffer_ptr[size()] = '\0';

		for (size_t k{0}; k < num_chars_erased; ++k)
			buffer_ptr[size() + 1 + k] = chars_to_remove[k];

		return buffer_ptr + offset;
	}

} // namespace dev

