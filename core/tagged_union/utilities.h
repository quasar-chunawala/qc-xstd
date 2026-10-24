#include <string>
#include <type_traits>
#include <utility>

namespace xstd {

	namespace tools {
		namespace type_lists {
			template <typename T, typename... Ts> constexpr size_t find_index_of_impl() {
				size_t i{0};
				for (bool is_same_type : {std::is_same_v<T, Ts>...}) {
					if (is_same_type)
						break;

					++i;
				}

				return i;
			}

			// define a concept that checks if the index is not
			// out of bounds
			template <typename T, typename... Ts>
			concept out_of_bounds_check = (find_index_of_impl<T, Ts...>() < sizeof...(Ts));
		} // namespace type_lists

		template <typename T, typename... Ts>
		requires type_lists::out_of_bounds_check<T, Ts...>
		constexpr size_t find_index_of_v = type_lists::find_index_of_impl<T, Ts...>();

		template <typename T, typename... Ts>
		concept exists = requires {
			{ find_index_of_v<T, Ts...> };
		};

	} // namespace tools

} // namespace xstd

// -------------- tests ------------
static_assert(xstd::tools::find_index_of_v<int, int, float, double> == 0);
static_assert(xstd::tools::find_index_of_v<float, int, float, double> == 1);
static_assert(xstd::tools::exists<double, int, float, std::string, double>);
static_assert(!xstd::tools::exists<std::string, int, float>);

#include <cstdint>

namespace xstd {

	namespace tools {
		namespace int_utils {
			template <size_t max> constexpr auto uint_atleast_impl() {
				if constexpr (max <= std::numeric_limits<std::uint8_t>::max())
					return uint8_t{};
				else if constexpr (max <= std::numeric_limits<std::uint16_t>::max())
					return uint16_t{};
				else if constexpr (max <= std::numeric_limits<std::uint32_t>::max())
					return uint32_t{};
				else if constexpr (max <= std::numeric_limits<std::uint64_t>::max())
					return uint64_t{};
			}
		} // namespace int_utils

		template <size_t max> using uint_atleast_t = decltype(int_utils::uint_atleast_impl<max>());

	} // namespace tools

} // namespace xstd

// -------------- tests ------------
#ifndef _MSC_VER
static_assert(std::is_same_v<uint8_t, xstd::tools::uint_atleast_t<100>>);
static_assert(std::is_same_v<uint8_t, xstd::tools::uint_atleast_t<255>>);
static_assert(std::is_same_v<uint16_t, xstd::tools::uint_atleast_t<256>>);
static_assert(std::is_same_v<uint32_t, xstd::tools::uint_atleast_t<std::numeric_limits<uint32_t>::max()>>);
static_assert(std::is_same_v<uint64_t, xstd::tools::uint_atleast_t<5'000'000'000>>);
#endif

namespace xstd {
	namespace tools {
		// Recursive approach
		namespace nth_element_type::v1 {
			namespace type_lists {
				template <size_t n, typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void,
				          typename T4 = void, typename T5 = void, typename T6 = void, typename T7 = void,
				          typename T8 = void, typename T9 = void, typename... Ts>
				constexpr auto get_nth_type_impl() {
					/**/ if constexpr (n == 0)
						return std::type_identity<T0>{};
					else if constexpr (n == 1)
						return std::type_identity<T1>{};
					else if constexpr (n == 2)
						return std::type_identity<T2>{};
					else if constexpr (n == 3)
						return std::type_identity<T3>{};
					else if constexpr (n == 4)
						return std::type_identity<T4>{};
					else if constexpr (n == 5)
						return std::type_identity<T5>{};
					else if constexpr (n == 6)
						return std::type_identity<T6>{};
					else if constexpr (n == 7)
						return std::type_identity<T7>{};
					else if constexpr (n == 8)
						return std::type_identity<T8>{};
					else if constexpr (n == 9)
						return std::type_identity<T9>{};
					else
						return get_nth_type_impl<n - 10, Ts...>();
				}
			} // namespace type_lists

		} // namespace nth_element_type::v1

		// Multiple inheritance trick
		namespace nth_element_type::v2 {
			template <size_t I, typename T> struct indexed {
				using type = T;
				static constexpr size_t index = I;
			};

			template <size_t I, typename T> using indexed_v = indexed<I, T>::index;

			template <typename Is, typename... Ts> struct indexer {};

			template <size_t... Is, typename... Ts>
			struct indexer<std::index_sequence<Is...>, Ts...> : indexed<Is, Ts>... {};

			// a static function
			template <size_t I, typename T> static indexed<I, T> select(indexed<I, T>);

			template <size_t I, typename... Ts>
			using get_nth_type_impl =
			    typename decltype(select<I>(indexer<std::index_sequence_for<Ts...>, Ts...>()))::type;

		} // namespace nth_element_type::v2

		// the void* trick
		namespace nth_element_type::v3 {
			template <size_t n, typename = std::make_index_sequence<n>> struct nth_type_impl;

			template <size_t n, size_t... ns> struct nth_type_impl<n, std::index_sequence<ns...>> {
				template <typename Tn> static Tn f(decltype((void *)ns)..., Tn *, void *...);
			};

			template <size_t n, typename... Ts>
			using nth_type_wrapper = decltype(nth_type_impl<n>::f((static_cast<Ts *>(nullptr))...));
		} // namespace nth_element_type::v3

		// The compiler intrinsic __nth_element
		// namespace nth_element_type::v4 {
		// 	template <size_t n, typename... Ts>
		// 	using get_nth_type_impl = __nth_element<n, Ts...>;
		// }

		template <size_t n, typename... Ts>
		requires(n < sizeof...(Ts))
		using get_nth_type_t = nth_element_type::v2::get_nth_type_impl<n, Ts...>;

	} // namespace tools

} // namespace xstd

// -------------- tests ------------
static_assert(std::is_same_v<xstd::tools::get_nth_type_t<0, char, short, int, long, float, double>, char>);

static_assert(std::is_same_v<xstd::tools::get_nth_type_t<5, char, short, int, long, float, double>, double>);

#include <vector>

namespace xstd {
	namespace tools {
		namespace _instance_of {
			template <template <typename...> typename Template, typename What>
			//       ^--------------------------------^       ^--------^
			//       1st param: a template that accepts       2nd param: a
			//       any number of args                       concrete type
			struct impl : std::false_type {};

			template <template <typename...> typename What, typename... Ts>
			struct impl<What, What<Ts...>> : std::true_type {};
		} // namespace _instance_of

		template <typename T, template <typename...> typename Template>
		concept instance_of = _instance_of::impl<Template, T>::value;
	} // namespace tools
} // namespace xstd

// -------------- tests ------------
static_assert(xstd::tools::instance_of<std::vector<int>, std::vector>);
static_assert(!xstd::tools::instance_of<int, std::vector>);
static_assert(!xstd::tools::instance_of<std::vector<int> &, std::vector>);

#define TOOLS_FWD(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)

namespace xstd {
	namespace tools {

		template <typename First, typename... Rest> struct union_ {
			union {
				First head;
				union_<Rest...> tail;
			};

			constexpr union_() {}
			constexpr ~union_() {}

			constexpr union_(std::in_place_index_t<0u>, auto &&...args) : head{TOOLS_FWD(args)...} {}

			template <size_t I>
			constexpr union_(std::in_place_index_t<I> idx, auto &&...args)
			    : tail{std::in_place_index<I - 1>, TOOLS_FWD(args)...} {}
		};

		template <typename T> struct union_<T> {
			T head;

			constexpr union_() {}
			constexpr ~union_() {}
			constexpr union_(std::in_place_index_t<0u>, auto &&...args) : head{TOOLS_FWD(args)...} {}
		};

	} // namespace tools
} // namespace xstd

namespace xstd {
	namespace tools {
		template <typename> struct union_size;

		template <typename... Ts> struct union_size<union_<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};

		template <typename Union> constexpr size_t union_size_v = union_size<Union>::value;
	} // namespace tools
} // namespace xstd

namespace xstd {
	namespace tools {
		namespace v1 {
			/**
			 * @brief Gets the nth element using the naive recursion
			 * approach.
			 *
			 * @tparam i
			 * @tparam Self
			 */
			template <std::size_t i, typename Self>
			constexpr auto &&get(Self &&self)
			requires instance_of<std::remove_cvref_t<Self>, union_> && (i < union_size_v<std::remove_cvref_t<Self>>)
			{
				if constexpr (i == 0)
					return TOOLS_FWD(self).head;
				else
					return get<i - 1>(TOOLS_FWD(self).tail);
			}
		} // namespace v1

		namespace v2 {
			// A second implementation technique consists in using
			// multiple inheriance and CTAD to trick the compiler to
			// deducing the nth element of a parameter pack for us.

			template <std::size_t I, typename T> struct indexed {
				using type = T;
			};

			template <typename Is, typename... Ts> struct indexer;

			template <std::size_t... Is, typename... Ts>
			struct indexer<std::index_sequence<Is...>, Ts...> : indexed<Is, Ts>... {};

			template <std::size_t I, typename T> static indexed<I, T> select(indexed<I, T>);

			template <size_t I, typename... Ts>
			using nth_element_type =
			    typename decltype(select<I>(indexer<std::index_sequence_for<Ts...>, Ts...>{}))::type;

			template <std::size_t I, typename... Ts>
			constexpr const nth_element_type<I, Ts...> &get(const union_<Ts...> &self) {
				return *(reinterpret_cast<const nth_element_type<I, Ts...> *>(&self));
			}

			template <std::size_t I, typename... Ts> constexpr nth_element_type<I, Ts...> &get(union_<Ts...> &self) {
				return *(reinterpret_cast<nth_element_type<I, Ts...> *>(&self));
			}
		} // namespace v2

		using v2::get;
	} // namespace tools
} // namespace xstd

namespace xstd {
	namespace tools {
		template <size_t i, typename... Ts> constexpr void construct_at(union_<Ts...> &self, auto &&...args) {
			// std::construct_at(<address>, <ctor args>...)
			std::construct_at(&self, std::in_place_index<i>, TOOLS_FWD(args)...);
		}

		template <size_t i, typename... Ts> constexpr void destroy_at(union_<Ts...> &self) {
			// std::destroy_at(<address>)
			std::destroy_at(&get<i>(self));
		}
	} // namespace tools
} // namespace xstd

namespace xstd {
	namespace tools {
		template <size_t I, typename T> struct indexed {
			using type = T;
			static constexpr size_t index = I;
		};

		template <size_t I, typename T> using indexed_v = indexed<I, T>::index;

		template <typename Is, typename... Ts> struct indexer {};

		template <size_t... Is, typename... Ts>
		struct indexer<std::index_sequence<Is...>, Ts...> : indexed<Is, Ts>... {};

		template <typename... Ts> using make_indexer = indexer<std::index_sequence_for<Ts...>, Ts...>;

	} // namespace tools
} // namespace xstd