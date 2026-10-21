#include <algorithm>
#include <cstddef>
#include <memory>
#include <meta>
#include <print>
#include <ranges>

#include <core/utilities/contract_assert.h>

// Godbolt: https://godbolt.org/z/44zasP8Y5

/// In real-life applications, such as an Entity-Component System(ECS), we often have aggregates that contain other
/// aggregate data-members.
///
/// Example.
///
/// struct Vec2{
///     double x_;
///     double y_;
/// };
///
/// struct Circle{
///     Vec2 position_;
///     double radius_;
/// };
///
/// For the purposes of gameplay, we may want to update the y-coordinate of 10'000 circles - shift the circles on the
/// Y-axis. Hence, the nested struct Vec2 should be stored in SoA fashion.
///
/// Counter-example.
/// struct Entity
/// {
///     struct Physics { Vec2 pos, vel; float torque; };
///     struct Rendering { Mesh* mesh; Texture* texture; };
///.
///.    Physics physics_;
///     Rendering rendering_;
/// };
///
/// A common access pattern in ECS is to store Physics and Rendering in AoS fashion, but Entity in SoA fasion.
///
/// xstd::soa_vector<T> implementation allows users to use [[=skip_flattening]]
/// annotation to opt-out of SoA for a nested type.
///
/// struct Entity
/// {
///     struct Physics { Vec2 pos, vel; float torque; };
///     struct Rendering { Mesh* mesh; Texture* texture; };
///.
///.    [[=skip_flatten]]Physics physics_;
///     [[=skip_flatten]]Rendering rendering_;
/// };

struct skip_flatten_t {};
static inline constexpr skip_flatten_t skip_flatten{};

namespace xstd {
	namespace rec_internal {
		namespace soa_utils {
#if __cpp_impl_reflection
			consteval void nsdms_of_impl(std::meta::info r_type, std::vector<std::meta::info> &data_members) {
				auto needs_flattening = [](std::meta::info r_member) consteval -> bool {
					return std::meta::annotations_of_with_type(r_member, ^^skip_flatten_t).empty();
				};
				for (auto mem : nonstatic_data_members_of(r_type, std::meta::access_context::unchecked())) {
					if (needs_flattening(mem)) {
						nsdms_of_impl(mem, data_members);
					} else {
						data_members.push_back(mem);
					}
				}
			}

			consteval auto nsdms_of(std::meta::info r_type) {
				std::vector<std::meta::info> data_members{};
				nsdms_of_impl(r_type, data_members);
				return data_members;
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
		} // namespace soa_utils

		template <typename T, typename Alloc = std::allocator<T>> struct soa_split_buffer;

		template <typename T, typename Alloc, typename Iterator> struct ConstructionTransaction {
			ConstructionTransaction(soa_split_buffer<T, Alloc> *parent, Iterator pos, Iterator end);

			ConstructionTransaction(soa_split_buffer<T, Alloc> *parent, Iterator pos, size_t n);

			~ConstructionTransaction();
			soa_split_buffer<T, Alloc> *parent_;
			Iterator pos_;
			const Iterator end_;

			friend class soa_split_buffer<T, Alloc>;
		};

		/// A soa_split_buffer is a contigous array data structure. It may hold spare capacity at both ends of the
		/// buffer. This allows for a soa_split_buffer to grow from both the front and the back without relocating its
		/// contents until it runs out of room.
		///
		/// The contents of the soa_vector are stored in a contigous buffer allocated on the heap. The contigous chunk
		/// of memory is delimited by the following pointers (o indicates uninitialized memory and x denotes valid
		/// objects).
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
				auto data_members = std::define_static_array(soa_utils::get_members_by_size<T>());
				define_aggregate(^^Pointers, data_members | std::views::transform([](std::meta::info member) {
					                             return data_member_spec(add_pointer(type_of(member)),
					                                                     {.name = identifier_of(member)});
				                             }));
				define_aggregate(^^ReferencesBase, data_members | std::views::transform([](std::meta::info member) {
					                                   return data_member_spec(add_lvalue_reference(type_of(member)),
					                                                           {.name = identifier_of(member)});
				                                   }));
			}

			static constexpr auto ptr_members = define_static_array(soa_utils::nsdms_of(^^pointers));
			static constexpr auto members = define_static_array(soa_utils::get_members_by_size<T>());

			/// TODO: Implement References::operator=() in a way that, if the aggregate type T contains
			/// aggregate members, which are also stored in SoA fashion, we should still be able to
			/// store it in a References object.
			struct References : ReferencesBase {
				auto operator=(const T &v) {
					template for (constexpr auto member : members) {
						[:identifier_of(member):] = v.[:identifier_of(member):];
					}
				}

				References *operator->() { return this; }
			};
#endif

			size_t begin_offset_{0};
			size_t size_{0};
			size_t capacity_{0};
			Pointers pointers_{};

#if __cpp_impl_reflection
			using tuple_pointers_t = [:std::meta::substitute(^^std::tuple,
			                                                 ptr_members | std::views::transform(std::meta::type_of) |
			                                                     std::ranges::to<std::vector>()):];
			using tuple_members_t = [:std::meta::substitute(^^std::tuple,
			                                                soa_utils::get_members_by_size<T>() |
			                                                    std::views::transform(std::meta::type_of) |
			                                                    std::ranges::to<std::vector>()):];
			using tuple_references_t = [:std::meta::substitute(
			                                 ^^std::tuple, soa_utils::get_members_by_size<T>() |
			                                                   std::views::transform(std::meta::type_of) |
			                                                   std::views::transform(std::meta::add_lvalue_reference) |
			                                                   std::ranges::to<std::vector>()):];
#else
			using tuple_pointers_t = decltype(soa_utils::tuple_type_of<Pointers>());
			using tuple_members_t = decltype(soa_utils::tuple_type_of<T>());
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

			constexpr void set_sentinel(iterator new_end) { end() = new_end; }

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

				ConstructionTransaction txn(this, end(), n);
			}

			/// KJ: Implement code that allocates, push_back a few elements
			/// so we are able to start testing the soa_vector at the earliest.
			///
		};

		template <typename T, typename Alloc, typename Iterator>
		ConstructionTransaction<T, Alloc, Iterator>::ConstructionTransaction(soa_split_buffer<T, Alloc> *parent,
		                                                                     Iterator pos, Iterator end)
		    : parent_{parent}, pos_{pos}, end_{end} {
		}

		template <typename T, typename Alloc, typename Iterator>
		ConstructionTransaction<T, Alloc, Iterator>::ConstructionTransaction(soa_split_buffer<T, Alloc> *parent,
		                                                                     Iterator pos, size_t n)
		    : parent_{parent}, pos_{pos}, end_{pos + n} {
		}

		template <typename T, typename Alloc, typename Iterator>
		ConstructionTransaction<T, Alloc, Iterator>::~ConstructionTransaction() {
			parent_->set_sentinel(pos_);
		}

		/// sizeof(T) * capacity

	} // namespace rec_internal

	template <typename T, typename Alloc = std::allocator<T>> class soa_vector_recursive {

		constexpr void grow() {
			// 1. allocate new storage
			// constexpr size_t growth_factor = 2;
			// size_t new_capacity = growth_factor * m_capacity;

			// 2. copy/move into the new storage
			// 3. deallocate the old storage
		}

	private:
		rec_internal::soa_split_buffer<T, Alloc> buffer_{};
	};
}; // namespace xstd
