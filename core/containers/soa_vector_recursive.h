#include <algorithm>
#include <cstddef>
#include <memory>
#include <meta>
#include <print>
#include <ranges>

#include <core/containers/soa_split_buffer.h>
#include <core/utilities/contract_assert.h>

// Godbolt: https://godbolt.org/z/44zasP8Y5

/// soa_vector_recursive<T>
/// -----------------------

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

	template <typename T, typename Alloc = std::allocator<T>> class soa_vector_recursive {};
}; // namespace xstd
