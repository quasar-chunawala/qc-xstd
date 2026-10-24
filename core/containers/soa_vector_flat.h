#ifndef SOA_VECTOR_FLAT_H
#define SOA_VECTOR_FLAT_H

#include <algorithm>
#include <cstddef>
#include <memory>
#include <meta>
#include <print>
#include <ranges>

#include <core/containers/soa_split_buffer.h>
#include <core/utilities/contract_assert.h>

// Godbolt: https://compiler-explorer.com/z/Kaq1aT9TE

namespace xstd {
	namespace flat_internal {

		template <typename T, typename Alloc = std::allocator<T>> class soa_vector_flat {
			using SplitBuffer = xstd::detail::soa_split_buffer<T, Alloc>;
			constexpr void grow() {
				// 1. allocate new storage
				// constexpr size_t growth_factor = 2;
				// size_t new_capacity = growth_factor * m_capacity;

				// 2. copy/move into the new storage
				// 3. deallocate the old storage
			}

		private:
			SplitBuffer buffer_{};
		};
	}; // namespace flat_internal
}; // namespace xstd

#endif