#ifndef SOA_UTILS_H
#define SOA_UTILS_H

#if __cpp_impl_reflection
	#include <meta>
#endif
#include <tuple>

namespace xstd::detail {

#if __cpp_impl_reflection
	consteval auto get_all_fields(std::meta::info reflected_type) {
		return nonstatic_data_members_of(reflected_type, std::meta::access_context::unchecked());
	}
#endif
	template <typename P> consteval auto tuple_type_of() {
		auto [... ptrs] = P{};
		return std::type_identity<std::tuple<decltype(ptrs)...>>{};
	}
} // namespace xstd::detail

#endif