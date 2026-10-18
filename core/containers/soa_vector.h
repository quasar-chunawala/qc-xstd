#include <algorithm>
#include <cstddef>
#include <memory>
#include <meta>
#include <print>
#include <ranges>

#include <core/utilities/contract_assert.h>

// Godbolt: https://godbolt.org/z/MsrYY1Ysb
namespace qc {
	namespace utils {
#if __cpp_impl_reflection
		consteval auto nsdms_of(std::meta::info type) {
			return nonstatic_data_members_of(type, std::meta::access_context::current());
		}

		template <typename T> consteval auto get_members_by_size() {
			auto data_members = nsdms_of(^^T);
			std::ranges::sort(data_members.begin(), data_members.end(), [](std::meta::info m1, std::meta::info m2) {
				return size_of(type_of(m1)) > size_of(type_of(m2));
			});
			return data_members;
		}
#endif
		template <typename P> consteval auto tuple_type_of() {
			auto [... ptrs] = P{};
			return std::type_identity<std::tuple<decltype(ptrs)...>>{};
		}
	} // namespace utils

	template <typename T, typename Alloc = std::allocator<T>> struct soa_split_buffer;

	template <typename T, typename Alloc, typename Pointers> struct ConstructionTransaction {
		ConstructionTransaction(soa_split_buffer<T, Alloc> *parent, Pointers pos, Pointers end)
		    : parent_{parent}, pos_{pos}, end_{end} {}

		soa_split_buffer<T, Alloc> *parent_;
		Pointers pos_;
		Pointers end_;
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

	template <typename T, typename Allocator> struct soa_split_buffer {
		using value_type = T;
		using size_type = size_t;
		using reference = T &;
		using const_reference = const T &;
		using pointer = T *;

		struct Pointers;
		struct ReferencesBase;
		struct References;

#if __cpp_impl_reflection
		consteval {
			auto data_members = std::define_static_array(utils::get_members_by_size<T>());
			define_aggregate(^^Pointers, data_members | std::views::transform([](std::meta::info member) {
				                             return data_member_spec(add_pointer(type_of(member)),
				                                                     {.name = identifier_of(member)});
			                             }));
			define_aggregate(^^ReferencesBase, data_members | std::views::transform([](std::meta::info member) {
				                                   return data_member_spec(add_lvalue_reference(type_of(member)),
				                                                           {.name = identifier_of(member)});
			                                   }));
		}

		static constexpr auto ptr_members = define_static_array(utils::nsdms(^^pointers));
		static constexpr auto members = define_static_array(utils::get_members_by_size<T>());

		struct References : ReferencesBase {
			auto operator=(const T &v) {
				template for (constexpr auto member : members) {
					[:identifier_of(member):] = v.[:identifier_of(member):];
				}
			}
		};
#endif

		size_t begin_offset_{0};
		size_t size_{0};
		size_t capacity_{0};
		Pointers pointers_{};

#if __cpp_impl_reflection
		using tuple_pointers_t = [:std::meta::substitute(^^std::tuple, ptr_members |
		                                                                   std::views::transform(std::meta::type_of) |
		                                                                   std::ranges::to<std::vector>()):];
		using tuple_members_t = [:std::meta::substitute(^^std::tuple, utils::get_members_by_size<T>() |
		                                                                  std::views::transform(std::meta::type_of) |
		                                                                  std::ranges::to<std::vector>()):];
		using tuple_references_t = [:std::meta::substitute(^^std::tuple,
		                                                   utils::get_members_by_size<T>() |
		                                                       std::views::transform(std::meta::type_of) |
		                                                       std::views::transform(std::meta::add_lvalue_reference) |
		                                                       std::ranges::to<std::vector>()):];
#else
		using tuple_pointers_t = decltype(utils::tuple_type_of<Pointers>());
		using tuple_members_t = decltype(utils::tuple_type_of<T>());
#endif
		struct iterator {
			using value_type = tuple_members_t;

			iterator operator++() {
				auto [... ptrs] = ptrs_;
				return iterator((ptrs + 1)...);
			}

			iterator operator++(int) {
				iterator temp = *this;
				++(*this);
				return temp;
			}

			iterator operator--() {
				auto [... ptrs] = ptrs_;
				return iterator(ptrs_ - 1);
			}

			iterator operator--(int) {
				iterator temp = *this;
				--(*this);
				return temp;
			}

			iterator operator+(std::ptrdiff_t n) {
				auto [... ptrs] = ptrs_;
				return iterator((ptrs + n)...);
			}

			iterator &operator+=(std::ptrdiff_t n) {
				auto [... ptrs] = ptrs_;
				*this = iterator((ptrs + n)...);
				return *this;
			}

			iterator operator-(std::ptrdiff_t n) {
				auto [... ptrs] = ptrs_;
				return iterator((ptrs - n)...);
			}

			iterator &operator-=(std::ptrdiff_t n) {
				auto [... ptrs] = ptrs_;
				*this = iterator((ptrs - n)...);
				return *this;
			}

			friend iterator operator+(std::ptrdiff_t n, const iterator &rhs) {
				iterator temp = rhs;
				return temp + n;
			}

			friend iterator operator-(std::ptrdiff_t n, const iterator &rhs) {
				iterator temp = rhs;
				return temp - n;
			}

			std::ptrdiff_t operator-(const iterator &rhs) {
				auto [... ptrs1] = ptrs_;
				auto [... ptrs2] = rhs.ptrs_;
				auto result = std::tie((ptrs1 - ptrs2)...);
				return std::get<0>(result);
			}

			References operator*() {
				auto [... ptrs] = ptrs_;
				return References((*ptrs)...);
			}

			References operator[](size_t idx) { return References(*(this + idx)); }

			References *operator->() {
				auto [... ptrs] = ptrs_;
				References ref = References((*ptrs)...);
				References *ref_ptr = &ref;
				return ref_ptr;
			}

			Pointers ptrs_;
		};

		constexpr iterator begin() {
			iterator front_cap(pointers_);
			return front_cap + begin_offset_;
		}

		constexpr iterator end() {
			iterator front_cap(pointers_);
			return front_cap + begin_offset_ + size_;
		}

		template <typename FieldType> constexpr auto get_field_span() {
			auto [... ptrs] = pointers_;
			auto tuple_pointers = std::make_tuple(ptrs...);
			FieldType *front_cap_ptr = std::get<FieldType>(tuple_pointers);
			return std::span(front_cap_ptr + begin_offset_, size_);
		}

		template <size_t N> constexpr auto get_field_span() {
			auto [... ptrs] = pointers_;
			auto tuple_pointers = std::make_tuple(ptrs...);
			auto *front_cap_ptr = std::get<N>(tuple_pointers);
			return std::span(front_cap_ptr + begin_offset_, size_);
		}

		constexpr size_t size() { return size_; }

		constexpr bool empty() { return size_ == 0; }

		constexpr size_t capacity() { return capacity_; }

		constexpr size_t front_spare() { return begin_offset_; }

		constexpr size_t back_spare() { return capacity_ - front_spare() - size_; }

		constexpr References front() {
			auto [... ptr] = pointers_;
			return References(*(ptr + begin_offset_)...);
		}

		constexpr References back() {
			auto [... ptr] = pointers_;
			return References(*(ptr + begin_offset_ + size_)...);
		}

		constexpr size_t get_sentinel() { return size_; }

		constexpr void set_sentinel(size_type size) { size_ = size; }

		constexpr size_t get_data() { return begin_offset_; }

		constexpr void set_data(size_t new_begin_offset_) { begin_offset_ = new_begin_offset_; }

		constexpr size_t set_valid_range(size_t new_begin_offset, size_t new_size) {
			begin_offset_ = new_begin_offset;
			size_ = new_size;
		}

		/// Copy constructs n objects starting at end()
		/// Pre-condition: n > 0
		/// Pre-condition: size() + n <= capacity()
		/// Post-condition: size() = old size() + n
		/// Post-condition: [i] == value for all i in [size() - n,size())
		constexpr void construct_at_end(size_type n, const_reference value) {
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(n > 0, "The number of objects to construct n, must satisfy n > 0");
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(size() + n <= capacity(),
			                                   "The container must have sufficient back spare capacity");
			XSTD_RECORD_OLD_VALUE(old_size, size());
			XSTD_CONTRACT_POSTCONDITION(size() == old_size + n, "The container size attribute must grow by n");
			for (auto i{size() - n}; i < size(); ++i) {
				XSTD_CONTRACT_POSTCONDITION(this[i] == value,
				                            "The element stored at [i] must equal user-supplied value");
			}
			auto [... mems] = value;
			auto [... ptrs] = pointers_;
			auto end = std::make_tuple((ptrs + begin_offset_ + size_)...);
			ConstructionTransaction txn(this, end, n);
		}

		/// KJ: Implement code that allocates, push_back a few elements
		/// so we are able to start testing the soa_vector at the earliest.
		///
	};
	/// sizeof(T) * capacity
	template <typename T, typename Alloc = std::allocator<T>> class soa_vector {

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