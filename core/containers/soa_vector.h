#include <algorithm>
#include <memory>
#include <meta>
#include <print>
#include <ranges>

namespace qc {
	namespace utils {
#if __cpp_impl_reflection
		consteval auto nsdms(std::meta::info type) {
			return nonstatic_data_members_of(
			    type, std::meta::access_context::current());
		}

		template <typename T> consteval auto get_members_by_size() {
			auto data_members = nsdms(^^T);
			std::ranges::sort(data_members.begin(),
			                  data_members.end(),
			                  [](std::meta::info m1, std::meta::info m2) {
				                  return size_of(type_of(m1)) >
				                         size_of(type_of(m2));
			                  });
			return data_members;
		}
#endif
	} // namespace utils

	template <typename T> struct soa_split_buffer;

	template <typename T> struct construct_transaction {
	private:
		soa_split_buffer<T> *parent;
	};

	/// A soa_split_buffer is a contigous array data structure. It may hold
	/// spare capacity at both ends of the buffer. This allows for a
	/// soa_split_buffer to grow from both the front and the back without
	/// relocating its contents until it runs out of room.
	///
	/// The contents of the soa_vector are stored in a contigous buffer
	/// allocated on the heap. The contigous chunk of memory is delimited
	/// by the following pointers (o indicates uninitialized memory and x
	/// denotes valid objects).
	///
	/// Suppose we have an aggregate type T{m_0, m_1, m_2}.
	///
	/// |ooooooooooooooxxxxxxxxooooooo|ooooooooooooxxxxxxxxxxxxxxxoooooooooooo|
	///  ^             ^       ^       ^           ^             ^ ^
	/// front_cap_0  begin_0  end_0  back_cap_0    begin_1       end_1
	/// back_cap_1
	///                              front_cap_1

	template <typename T> struct soa_split_buffer {
		using value_type = T;
		using size_type = size_t;
		using reference = T &;
		using const_reference = const T &;
		using pointer = T *;

		struct pointers;
#if __cpp_impl_reflection
		consteval {
			auto data_members =
			    std::define_static_array(utils::get_members_by_size<T>());
			define_aggregate(
			    ^^pointers,
			    data_members |
			        std::views::transform([](std::meta::info member) {
				        return data_member_spec(
				            add_pointer(type_of(member)),
				            {.name = identifier_of(member)});
			        }));
		}
#endif
		size_t begin_offset_{0};
		size_t size_{0};
		size_t capacity_{0};
		pointers pointers_{};

		constexpr auto begin() {
			auto [... ptr] = pointers_;
			return std::make_tuple((ptr + begin_offset_)...);
		}

		constexpr auto end() {
			auto [... ptr] = pointers_;
			return std::make_tuple((ptr + begin_offset_ + size_)...);
		}

		constexpr size_t size() { return size_; }

		constexpr size_t capacity() { return capacity_; }

		constexpr size_t front_spare() { return begin_offset_; }

		constexpr size_t back_spare() {
			return capacity_ - front_spare() - size_;
		}

		constexpr auto front() {
			auto [... ptr] = pointers_;
			return std::tie(*(ptr + begin_offset_)...);
		}

		constexpr void set_sentinel(size_type size) { size_ = size; }

		/// Copy constructs n objects starting at end()
		/// Pre-condition: n > 0
		/// Pre-condition: size() + n <= capacity()
		/// Post-condition: size() = old size() + n
		/// Post-condition: [i] == value for all i in [size() - n,size())
		constexpr void construct_at_end(size_type n,
		                                const_reference value) {
			auto [... mems] = value;
			auto [... ptrs] = pointers_;
			auto end = std::make_tuple((ptrs + begin_offset_ + size_)...);
			construct_transaction txn(this, end, n);
		}

		/// KJ: Implement code that allocates, push_back a few elements
		/// so we are able to start testing the soa_vector at the earliest.
		///
	};
	/// sizeof(T) * capacity
	template <typename T, typename Alloc = std::allocator<T>>
	class soa_vector {

		constexpr void grow() {
			// 1. allocate new storage
			// constexpr size_t growth_factor = 2;
			// size_t new_capacity = growth_factor * m_capacity;

			// 2. copy/move into the new storage
			// 3. deallocate the old storage
		}

	private:
		soa_split_buffer<T> buffer_{};
	};
} // namespace qc