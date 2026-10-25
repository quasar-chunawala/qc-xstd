#ifndef SOA_SPLIT_BUFFER_H
#define SOA_SPLIT_BUFFER_H

#include <cstdint>
#include <memory>
#include <ranges>
#include <tuple>

#include <core/containers/construction_transaction.h>
#include <core/containers/soa_utils.h>
#include <core/utilities/contract_assert.h>

namespace xstd::detail {
	/**
	 * @brief An internal storage for the soa_vector.
	 *
	 * @details @a soa_split_buffer is a contiguous array data structure. It may
	 * hold spare capacity at both ends of the buffer. This allows a
	 * soa_split_buffer to grow from both the front and the back without relocating
	 * its contents until it runs out of room.
	 *
	 * The contents of the soa_split_buffer are stored in a contiguous buffer
	 * allocated on the heap. For an aggregate type `T{m_0, m_1, m_2}`, each member
	 * is stored in its own sub-array, delimited by the pointers shown below (`o`
	 * marks uninitialized memory and `x` marks valid objects).
	 *
	 * @verbatim
	 * |ooooooooooooooxxxxxxxxooooooo|ooooooooooooxxxxxxxxxxxxxxxoooooooooooo|
	 *  ^             ^       ^       ^           ^              ^           ^
	 *  front_cap_0   |       end_0   |           begin_1        | back_cap_1
	 *                begin_0         back_cap_0 = front_cap_1   end_1
	 *  @endverbatim
	 *
	 */

	template <typename T, typename Allocator> struct soa_split_buffer {
		using value_type = T;
		using size_type = std::size_t;
		using reference = T &;
		using const_reference = const T &;
		using pointer = T *;

		struct Pointers;
		struct ReferencesBase;
		struct References;

#if __cpp_impl_reflection
		consteval {
			auto data_members = std::define_static_array(get_all_fields(^^T));
			define_aggregate(^^Pointers, data_members | std::views::transform([](std::meta::info member) {
				                             return data_member_spec(add_pointer(type_of(member)),
				                                                     {.name = identifier_of(member)});
			                             }));
			define_aggregate(^^ReferencesBase, data_members | std::views::transform([](std::meta::info member) {
				                                   return data_member_spec(add_lvalue_reference(type_of(member)),
				                                                           {.name = identifier_of(member)});
			                                   }));
		}

		static constexpr auto ptr_members = define_static_array(get_all_fields(^^Pointers));
		static constexpr auto members = define_static_array(get_all_fields(^^T));

		struct References : ReferencesBase {
			References &operator=(const T &v) {
				template for (constexpr auto member : members) {
					[:identifier_of(member):] = v.[:identifier_of(member):];
				}
				return *this;
			}

			References *operator->() { return this; }
		};
#endif
		Pointers pointers_{};
		std::size_t begin_offset_{0};
		std::size_t size_{0};
		std::size_t capacity_{0};
		[[no_unique_address]] Allocator alloc_;

#if __cpp_impl_reflection
		using tuple_pointers_t = [:std::meta::substitute(^^std::tuple, ptr_members |
		                                                                   std::views::transform(std::meta::type_of) |
		                                                                   std::ranges::to<std::vector>()):];
		using tuple_members_t = [:std::meta::substitute(^^std::tuple, members |
		                                                                  std::views::transform(std::meta::type_of) |
		                                                                  std::ranges::to<std::vector>()):];
		using tuple_references_t = [:std::meta::substitute(^^std::tuple,
		                                                   members | std::views::transform(std::meta::type_of) |
		                                                       std::views::transform(std::meta::add_lvalue_reference) |
		                                                       std::ranges::to<std::vector>()):];
#else
		using tuple_pointers_t = decltype(tuple_type_of<Pointers>());
		using tuple_members_t = decltype(tuple_type_of<T>());
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

			friend iterator operator+(std::ptrdiff_t n, const iterator &rhs) { return rhs + n; }

			friend iterator operator-(std::ptrdiff_t n, const iterator &rhs) { return rhs - n; }

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

			References operator[](std::size_t idx) { return References(*(this + idx)); }

			References *operator->() {
				auto [... ptrs] = ptrs_;
				References ref = References((*ptrs)...);
				return ref;
			}

			iterator operator=(iterator &rhs) {
				ptrs_ = rhs.ptrs_;
				return *this;
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

		template <std::size_t N> constexpr auto get_field_span() {
			auto [... ptrs] = pointers_;
			auto tuple_pointers = std::make_tuple(ptrs...);
			auto *front_cap_ptr = std::get<N>(tuple_pointers);
			return std::span(front_cap_ptr + begin_offset_, size_);
		}

		[[nodiscard]] constexpr std::size_t size() const { return size_; }

		[[nodiscard]] constexpr bool empty() const { return size_ == 0; }

		[[nodiscard]] constexpr std::size_t capacity() const { return capacity_; }

		[[nodiscard]] constexpr std::size_t front_spare() const { return begin_offset_; }

		[[nodiscard]] constexpr std::size_t back_spare() const { return capacity_ - front_spare() - size_; }

		constexpr References front() {
			auto [... ptr] = pointers_;
			return References(*(ptr + begin_offset_)...);
		}

		constexpr References back() {
			auto [... ptr] = pointers_;
			return References(*(ptr + begin_offset_ + size_)...);
		}

		[[nodiscard]] constexpr std::size_t get_sentinel() const { return size_; }

		constexpr void set_sentinel(size_type size) { size_ = size; }

		constexpr void set_sentinel(iterator new_end) { end() = new_end; }

		[[nodiscard]] constexpr std::size_t get_data() const { return begin_offset_; }

		constexpr void set_data(size_type new_begin_offset_) { begin_offset_ = new_begin_offset_; }

		constexpr void set_valid_range(size_type new_begin_offset, size_type new_size) {
			begin_offset_ = new_begin_offset;
			size_ = new_size;
		}

		/**
		 * @brief Copy constructs n objects starting at end().
		 *
		 * @pre n > 0
		 * @pre size() + n <= capacity()
		 * @post size() = old_size + n
		 * @post [i] == value for all i in [size() - n, size())
		 */
		constexpr void copy_construct_at_end(size_type n, const_reference value) {
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(n > 0, "The number of objects to construct n, must satisfy n > 0");
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(size() + n <= capacity(),
			                                   "The container must have sufficient back spare capacity");
			XSTD_RECORD_OLD_VALUE(old_size, size());
			XSTD_CONTRACT_POSTCONDITION(size() == old_size + n, "The container size attribute must grow by n");

			// RAII wrapper that provides basic exception guarantee when constructing value n times.
			ConstructionTransaction txn(this, end(), n);
			for (; txn.pos_ != txn.end_; ++txn.pos_) {
				auto [... member_ptrs] = pointers_;
				auto &[... member_values] = value;
				// RAII guard that provides that offers strong exception safety, when constructing a single
				// T element, in the member declaration order.
				SOAVecElementConstructionTransaction<Allocator, decltype(member_values)...> elementTxn(
				    member_values..., member_ptrs..., alloc_);
			}
		}

		/**
		 * @brief Determine the size in bytes for an allocation request for the container to have
		 * enough space for being able to hold requested_capacity elements of type T in the future.
		 * @param requested_capacity The requested spare capacity.
		 * @return The number of bytes of raw memory storage required.
		 */
		constexpr size_t required_bytes(size_t requested_capacity) {
			size_t required_bytes = 0;
#if __cpp_impl_reflection
			static constexpr int num_members = members.size();
			template for (constexpr int I : std::views::iota(0, num_members)) {
				size_t align_requirement = std::meta::alignment_of(type_of(members[I]));
				if constexpr (I > 0) {
					size_t padding = required_bytes & (align_requirement - 1);
					padding = padding ? align_requirement - padding : 0;
					required_bytes += padding;
				}
				required_bytes += size_of(type_of(members[I])) * requested_capacity;
			}
#endif
			return required_bytes;
		}

		/**
		 * @brief Determine the alignment boundary
		 *
		 * @return constexpr size_t
		 */
		constexpr size_t required_alignment() {
			size_t align_at_boundary{alignof(T)};
#if __cpp_impl_reflection
			align_at_boundary = std::meta::alignment_of(type_of(members[0]));
#endif
			return align_at_boundary;
		}

		constexpr std::byte *allocate(size_t capacity) {}

		/**
		 * @brief Default constructor.
		 */
		constexpr soa_split_buffer() = default;

		constexpr soa_split_buffer(size_type start, size_type capacity) {
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(capacity > 0, "The capacity must be greater than 0");
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(start >= 0 && start < capacity,
			                                   "Cannot have a start point outside the capacity");
		}
	};
} // namespace xstd::detail

#endif