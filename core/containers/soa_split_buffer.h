#ifndef SOA_SPLIT_BUFFER_H
#define SOA_SPLIT_BUFFER_H

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <print>
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

	template <typename T, typename Allocator = std::allocator<T>> struct soa_split_buffer {
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

		/**
		 * @brief StorageBlock type is used by the allocate_memory functions as a unit memory allocation.
		 */
		struct alignas(T) StorageBlock {
			std::byte byte_array[alignof(T)];
		};

		Pointers pointers_{};
		StorageBlock *raw_memory_{nullptr};
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
			using iterator_category = std::contiguous_iterator_tag;
			using iterator_type = Pointers;
			using value_type = References;
			using pointer = Pointers;
			using difference_type = std::ptrdiff_t;

			constexpr iterator operator++() {
				auto [... ptrs] = ptrs_;
				return iterator(Pointers{(ptrs + 1)...});
			}

			constexpr iterator operator++(int) {
				iterator temp = *this;
				++(*this);
				return temp;
			}

			constexpr iterator operator--() {
				auto [... ptrs] = ptrs_;
				return iterator(Pointers{(ptrs - 1)...});
			}

			constexpr iterator operator--(int) {
				iterator temp = *this;
				--(*this);
				return temp;
			}

			constexpr iterator operator+(std::ptrdiff_t n) {
				auto [... ptrs] = ptrs_;
				return iterator(Pointers{(ptrs + n)...});
			}

			constexpr iterator &operator+=(std::ptrdiff_t n) {
				auto [... ptrs] = ptrs_;
				*this = iterator(Pointers{(ptrs + n)...});
				return *this;
			}

			constexpr iterator operator-(std::ptrdiff_t n) {
				auto [... ptrs] = ptrs_;
				// auto [... result_ptrs] = ((ptrs - n), ...);
				return iterator(Pointers{(ptrs - n)...});
			}

			constexpr iterator &operator-=(std::ptrdiff_t n) {
				auto [... ptrs] = ptrs_;
				*this = iterator(Pointers{(ptrs - n)...});
				return *this;
			}

			friend constexpr iterator operator+(std::ptrdiff_t n, const iterator &rhs) { return rhs + n; }

			friend constexpr iterator operator-(std::ptrdiff_t n, const iterator &rhs) { return rhs - n; }

			constexpr std::ptrdiff_t operator-(const iterator &rhs) {
				auto [lhs_first, ... ptrs1] = ptrs_;
				auto [rhs_first, ... ptrs2] = rhs.ptrs_;
				return lhs_first - rhs_first;
			}

			constexpr References operator*() {
				auto [... ptrs] = ptrs_;
				return References((*ptrs)...);
			}

			constexpr References operator[](std::size_t idx) { return References(*(this + idx)); }

			constexpr References *operator->() {
				auto [... ptrs] = ptrs_;
				References ref = References((*ptrs)...);
				return ref;
			}

			constexpr iterator operator=(iterator &rhs) {
				ptrs_ = rhs.ptrs_;
				return *this;
			}

			constexpr bool operator==(iterator &rhs) {
				auto [... lhs_ptrs] = ptrs_;
				auto [... rhs_ptrs] = rhs.ptrs_;
				return ((lhs_ptrs == rhs_ptrs) && ...);
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

		constexpr void set_size(size_t new_size) { size_ = new_size; }

		[[nodiscard]] constexpr bool empty() const { return size_ == 0; }

		[[nodiscard]] constexpr std::size_t capacity() const { return capacity_; }

		[[nodiscard]] constexpr std::size_t front_spare() const { return begin_offset_; }

		constexpr void set_front_spare(size_t new_begin_offset) { begin_offset_ = new_begin_offset; }

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

		constexpr void set_sentinel(iterator new_end) {
			auto [begin_ptr, ... begin_ptrs] = begin().ptrs_;
			auto [end_ptr, ... end_ptrs] = new_end.ptrs_;
			set_size(end_ptr - begin_ptr + 1);
		}

		constexpr void set_capacity(size_type new_capacity) { capacity_ = new_capacity; }

		[[nodiscard]] constexpr std::size_t get_data() const { return begin_offset_; }

		constexpr void set_data(size_type new_begin_offset_) { begin_offset_ = new_begin_offset_; }

		constexpr void set_valid_range(size_type new_begin_offset, size_type new_size) {
			begin_offset_ = new_begin_offset;
			size_ = new_size;
		}

		/**
		 * @brief Sets the split_buffer's raw_memory_ byte pointer to the starting address of the allocated memory.
		 * The split_buffer's contigous storage is divided into sub-arrays, one each for T's member values.
		 * pointers_ field is initialized to the base addresses of each of the sub-arrays.
		 *
		 * @param raw_mem Starting address of the allocated memory on the heap.
		 * @param capacity
		 */
		constexpr void set_pointers(std::byte *raw_mem, size_type capacity) {
			raw_memory_ = reinterpret_cast<StorageBlock *>(raw_mem);
#if __cpp_impl_reflection
			const auto member_offsets = calculate_member_offsets<T>(capacity);
			std::size_t i = 0;
			template for (constexpr auto ptr_member : ptr_members) {
				pointers_.[:ptr_member:] = reinterpret_cast<[:type_of(ptr_member):]>(raw_mem +
				                                                                     member_offsets[i++].offset);
			}
#endif
			set_capacity(capacity);
		}

		/**
		 * @brief Copy constructs n objects starting at end().
		 *
		 * @pre n > 0
		 * @pre size() + n <= capacity()
		 * @post size() = old_size + n
		 * @post [i] == value for all i in [size() - n, size())
		 */
		constexpr void copy_construct_at_end_with_size(size_type n, const_reference value) {
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(n > 0, "The number of objects to construct n, must satisfy n > 0");
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(size() + n <= capacity(),
			                                   "The container must have sufficient back spare capacity");
			XSTD_RECORD_OLD_VALUE(old_size, size());
			XSTD_CONTRACT_POSTCONDITION(size() == old_size + n, "The container size attribute must grow by n");

			// RAII wrapper that provides basic exception guarantee when constructing value n times.
			ConstructionTransaction txn(this, end(), n);
			for (; txn.pos_ != txn.end_; ++txn.pos_) {
				auto [... member_ptrs] = txn.pos_.ptrs_;
				auto &[... member_values] = value;
				// RAII guard that provides that offers strong exception safety, when constructing a single
				// T element, in the member declaration order.
				SOAVecElementConstructionTransaction<Allocator, decltype(member_values)...> elementTxn(
				    member_values..., member_ptrs..., alloc_);
			}
			set_size(size() + n);
		}

		/**
		 * @brief Copy constructs the range [first,last) at the end.
		 *
		 * @details This method accepts a pair of forward iterators. Hence, we know the number of elements
		 *          in the range, and we expect the caller to ensure that there is enough room in the soa_split_buffer.
		 * @tparam ForwardIterator
		 * @param first The start of the range.
		 * @param last One past the end of the range.
		 */
		template <std::forward_iterator ForwardIterator>
		constexpr void copy_construct_at_end_with_size(ForwardIterator first, ForwardIterator last) {
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(last > first,
			                                   "The number of objects to construct n, must satisfy n > 0");
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(size() + std::distance(first, last),
			                                   "The container must have sufficient back spare capacity");
			// RAII wrapper that provides basic exception guarantee when constructing n elements.
			ConstructionTransaction txn(this, end(), std::distance(first, last));
			auto src_iter{first};
			for (; txn.pos_ != txn.end_; ++txn.pos_) {
				auto [... member_ptrs] = txn.pos_.ptrs_;
				auto &[... member_values] = *src_iter;
				// RAII guard that provides that offers strong exception safety, when constructing a single
				// T element, in the member declaration order.
				SOAVecElementConstructionTransaction<Allocator, decltype(member_values)...> elementTxn(
				    member_values..., member_ptrs..., alloc_);
			}
			set_size(size() + std::distance(first, last));
		}

		template <std::input_iterator InputIterator, typename Sentinel>
		constexpr void construct_at_end_with_sentinel(InputIterator first, Sentinel last) {

			while (first != last) {
				if (!back_spare()) {
					// We ran out of back spare while reading the input stream.
					size_t old_capacity = capacity();
					size_t new_capacity = std::max({2 * old_capacity, 8ul});
					soa_split_buffer new_buf(new_capacity, 0);

					auto dest_iter = new_buf.begin();
					auto src_iter = begin();
					// If the relocation throws mid-way, it guarantees that
					// new_buf will be in a valid state. If an exception is thrown in the below for-loop block, if we
					// exit the scope prematurely, new_buf will automatically clear its elements upon destruction.
					size_t new_buf_size{0};
					for (; src_iter != end(); ++src_iter, ++dest_iter) {
						auto [... member_ptrs] = dest_iter.ptrs_;
						auto &[... member_values] = *src_iter;
						// RAII guard that provides that offers strong exception safety, when constructing a single
						// T element, in the member declaration order.
						SOAVecElementConstructionTransaction<Allocator, decltype(member_values)...> elementTxn(
						    std::move(member_values)..., member_ptrs..., alloc_);

						new_buf.set_sentinel(++new_buf_size);
					}
					swap(*this, new_buf);
				}
				auto [... member_ptrs] = end().ptrs_;
				auto &[... member_values] = *first;
				// RAII guard that provides that offers strong exception safety, when constructing a single
				// T element, in the member declaration order.
				SOAVecElementConstructionTransaction<Allocator, decltype(member_values)...> elementTxn(
				    member_values..., member_ptrs..., alloc_);
				set_sentinel(++size_);
				++first;
			}
		}

		friend void swap(soa_split_buffer &lhs, soa_split_buffer &rhs) noexcept {
			using std::swap;
			swap(lhs.raw_memory_, rhs.raw_memory_);
			swap(lhs.pointers_, rhs.pointers_);
			swap(lhs.begin_offset_, rhs.begin_offset_);
			swap(lhs.size_, rhs.size_);
			swap(lhs.capacity_, rhs.capacity_);
		}

		/**
		 * @brief Destructs objects at new_end and onwards.
		 *
		 * @param new_end
		 */
		constexpr void destruct_at_end(iterator new_end) {
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(new_end >= begin() && new_end < end(),
			                                   "new_last must in [begin(),end())");
			XSTD_CONTRACT_POSTCONDITION(size_ == new_end - begin(),
			                            "The container size attribute must be updated to end() - begin()");
			auto iter = end();
			while (iter != new_end) {
				--iter;
				auto [... ptrs] = iter.ptrs_;
				(std::destroy_at(ptrs), ...);
			}
			size_ = (new_end - begin());
		}

		constexpr void clear() { destruct_at_end(begin()); }

		/**
		 * @brief Helper function that calculates the allocation size and obtains
		 *.memory on the heap large enough to hold capacity elements of type T in SoA fashion,
		 * and returns a handle to this block.
		 *
		 * @param capacity The user-requested capacity for the container.
		 */
		constexpr std::byte *allocate_memory(size_t capacity) {
			static constexpr size_t alignment = alignof(T);
			size_t nbytes = calculate_allocation_size<T>(capacity);

			// Rebind the allocator
			using traits = typename std::allocator_traits<Allocator>::template rebind_traits<StorageBlock>;
			using byte_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<StorageBlock>;

			byte_allocator_type byte_allocator(alloc_);
			size_t num_blocks = (nbytes + alignment - 1) / alignment;
			StorageBlock *block_ptr = traits::allocate(byte_allocator, num_blocks);
			// std::println("Allocated {} bytes from {} to {}",
			//              num_blocks * sizeof(StorageBlock),
			//              raw_memory_,
			//              raw_memory_ + (num_blocks * sizeof(StorageBlock)));
			return reinterpret_cast<std::byte *>(block_ptr);
		}

		constexpr void deallocate_memory() {
			static constexpr size_t alignment = alignof(T);
			size_t nbytes = calculate_allocation_size<T>(capacity_);

			// Rebind the allocator
			using traits = typename std::allocator_traits<Allocator>::template rebind_traits<StorageBlock>;
			using byte_allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<StorageBlock>;
			byte_allocator_type byte_allocator(alloc_);
			size_t num_blocks = (nbytes + alignment - 1) / alignment;

			traits::deallocate(byte_allocator, raw_memory_, num_blocks);
			// std::println("Deallocated {} bytes from {} to {}",
			//              num_blocks * sizeof(StorageBlock),
			//              raw_memory_,
			//              raw_memory_ + (num_blocks * sizeof(StorageBlock)));
		}

		/**
		 * @brief Default constructor.
		 */
		constexpr soa_split_buffer() = default;

		/**
		 * @brief Constructor that accepts a capacity and begin_offset.
		 *
		 */
		constexpr soa_split_buffer(size_type capacity, size_type begin_offset) {
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(capacity > 0, "The capacity must be greater than 0");
			XSTD_ASSERT_ARGUMENT_WITHIN_DOMAIN(start >= 0 && start < capacity,
			                                   "Cannot have a start point outside the capacity");
			std::byte *raw_mem = allocate_memory(capacity);
			set_pointers(raw_mem, capacity);
			set_valid_range(begin_offset, 0);
		}

		/**
		 * @brief Destroy the split buffer object
		 *
		 */
		constexpr ~soa_split_buffer() {
			clear();
			if (raw_memory_) {
				deallocate_memory();
				raw_memory_ = nullptr;
			}
		}
	};
} // namespace xstd::detail

#endif