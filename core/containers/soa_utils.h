#ifndef SOA_UTILS_H
#define SOA_UTILS_H

#include <array>
#include <cstddef>
#include <ranges>
#include <span>
#if __cpp_impl_reflection
#include <meta>
#endif
#include <cstdint>
#include <tuple>

namespace xstd::detail {

#if __cpp_impl_reflection
consteval auto get_all_fields(std::meta::info reflected_type) {
  return nonstatic_data_members_of(reflected_type,
                                   std::meta::access_context::unchecked());
}
#endif

/**
 * @brief Round offset up to the nearest multiple of alignment.
 * @param offset
 * @param alignment A power of 2.
 * @details In general x & (~x) isolates the lowest set bit of x.
 * @verbatim
 * If x is a power of 2, for example, let x := 0b0000 1000 (2^3), then :
 *
 *		   (x - 1)  -->  0b0000 0111
 *
 *  Example 1. v := 5.
 *     v + (x - 1)  -->  0b0000 1100
 *		 & (~x + 1) -->  0b1111 1000
 * ---------------------------------
 *                       0b0000 1000
 *
 *  Example 2. v := 8.
 *     v + (x - 1)  -->  0b0000 1111
 *		 & (~x + 1) -->  0b1111 1000
 * ---------------------------------
 *                       0b0000 1000
 *
 *  Example 3. v := 9.
 *     v + (x - 1)  -->  0b0001 0000
 *		 & (~x + 1) -->  0b1111 1000
 * ---------------------------------
 *                       0b0001 0000
 *
 * (v + (x - 1)) & (~x + 1) gives v rounded up to the nearest multiple
 * of x.
 * @endverbatim
 * @return
 */
constexpr std::size_t align_up(std::size_t offset, std::size_t alignment) {
  return (offset + alignment - 1) & (~alignment + 1);
}

/**
 * @brief Determine the offset in bytes for the array of values of type
 * MemberType in the soa_split_buffer.
 * @tparam MemberType
 * @param offset The first free byte after the previous sub-array.
 */
template <typename MemberType>
constexpr std::size_t calculate_allocation_offset(std::size_t offset) {
  return align_up(offset, alignof(MemberType));
}

#if __cpp_impl_reflection
struct member_layout {
  std::size_t size;
  std::size_t alignment;
};

struct member_offset {
  std::size_t size;
  std::size_t alignment;
  std::size_t offset;
  std::size_t trailing_padding_bytes;
};

/**
 * @brief The size and alignment of each non-static data member of T, in
 * declaration order.
 */
template <typename T> consteval auto member_layouts_of() {
  constexpr auto members = define_static_array(get_all_fields(^^T));
  static constexpr size_t num_members = members.size();
  std::array<member_layout, num_members> layouts{};
  template for (constexpr size_t I : std::views::iota(0ul, num_members)) {
    layouts[I].size = size_of(type_of(members[I]));
    layouts[I].alignment = alignment_of(type_of(members[I]));
  }
  return layouts;
}

template <typename T>
inline constexpr auto member_layouts = member_layouts_of<T>();

/**
 * @brief Byte offset of each member's sub-array from the start of the
 * allocation, for a soa_split_buffer<T> holding capacity elements.
 */
template <typename T>
constexpr auto calculate_member_offsets(std::size_t capacity) {
  constexpr auto members = define_static_array(get_all_fields(^^T));
  auto layouts = member_layouts<T>;
  static constexpr size_t num_members = members.size();

  std::array<member_offset, num_members> member_offsets{};
  size_t allocation_offset{0};
  template for (constexpr auto I : std::views::iota(0ul, num_members)) {
    size_t align_requirement = layouts[I].alignment;
    constexpr auto calculate_allocation_offset_func_reflection =
        substitute(^^calculate_allocation_offset, {
                                                      type_of(members[I])});
    size_t old_offset = allocation_offset;
    allocation_offset = [:calculate_allocation_offset_func_reflection:](
        allocation_offset);
    if constexpr (I > 0) {
      member_offsets[I - 1].trailing_padding_bytes =
          allocation_offset - old_offset;
    }
    member_offsets[I].size = layouts[I].size;
    member_offsets[I].alignment = layouts[I].alignment;
    member_offsets[I].offset = allocation_offset;
    allocation_offset += layouts[I].size * capacity;
  }
  return member_offsets;
}

/**
 * @brief Determine the size in bytes for an allocation request for the
 * container to have enough space for being able to hold capacity elements of
 * type T in the future.
 * @param capacity The requested spare capacity.
 * @return The number of bytes of raw memory storage required.
 */
template <typename T>
constexpr std::size_t calculate_allocation_size(std::size_t capacity) {
  auto allocation_offsets = calculate_member_offsets<T>(capacity);
  if (allocation_offsets.empty()) {
    return 0;
  }

  return allocation_offsets[allocation_offsets.size() - 1].offset +
         allocation_offsets[allocation_offsets.size() - 1].size * capacity;
}
#endif

template <typename P> consteval auto tuple_type_of() {
  auto [... ptrs] = P{};
  return std::type_identity<std::tuple<decltype(ptrs)...>>{};
}
} // namespace xstd::detail

#endif