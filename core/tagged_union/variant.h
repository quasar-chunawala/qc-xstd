#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include <core/tagged_union/utilities.h>

namespace xstd {

	template <typename... Ts> class variant;

	// Forward declaration
	// so the friend declaration inside variant resolves
	template <size_t I, typename... Ts> constexpr decltype(auto) get(const variant<Ts...> &v);

	template <typename T, typename... Ts> constexpr T &get(const variant<Ts...> &v);

	template <typename... Ts> class variant {
		using index_type = tools::uint_atleast_t<sizeof...(Ts)>;
		static constexpr size_t size = sizeof...(Ts);

		tools::union_<Ts...> m_data;
		index_type m_idx;

	public:
		/**
		 * @brief: Constructs a variant holding the zero-initialized
		 * value of the first alternative.
		 */
		constexpr variant() {
			tools::construct_at<0>(m_data, tools::get_nth_type_t<0, Ts...>());
			m_idx = 0;
		}

		/**
		 * @brief Converting constructor. Constructs a variant holding
		 * the alternative of type @p T, from @p x.
		 */
		template <tools::exists<Ts...> T> constexpr explicit variant(T &&x) {
			tools::construct_at<tools::find_index_of_v<T, Ts...>>(m_data, TOOLS_FWD(x));
			m_idx = tools::find_index_of_v<T, Ts...>;
		}

		/**
		 * @brief Copy constructor. Constructs a variant holding the
		 * same alternative as @p other, copy-constructed from the
		 * contained value.
		 */
		constexpr variant(const variant &other) {
			index_type idx = other.m_idx;
			constexpr auto idx_seq = std::make_index_sequence<sizeof...(Ts)>();
			[&]<auto... Indices>(std::index_sequence<Indices...>) {
				(((idx == Indices) ? (tools::construct_at<Indices>(m_data, tools::get<Indices>(other.m_data)), 0) : 0),
				 ...);
			}(idx_seq);
			m_idx = idx;
		}

		/**
		 * @brief Move constructor. Steals data from @p other.
		 * Constructs a variant holding the same
		 * alternative as @p other, move-constructed from
		 * the contained value.
		 */
		constexpr variant(variant &&other) noexcept {
			index_type idx = other.m_idx;
			constexpr auto idx_seq = std::make_index_sequence<sizeof...(Ts)>();
			[&]<auto... Indices>(std::index_sequence<Indices...>) {
				(((idx == Indices)
				      ? (tools::construct_at<Indices>(m_data, tools::get<Indices>(std::move(other.m_data))), 0)
				      : 0),
				 ...);
			}(idx_seq);
			m_idx = idx;
		}

		/**
		 * @brief Swaps the contents of two variants.
		 *
		 * If both variants hold the same alternative (index() ==
		 * other.index()), the contained values are swapped directly
		 * via @c std::swap. Otherwise, the alternatives are exchanged
		 * by using a temporary.
		 */
		void swap(variant<Ts...> &other) noexcept {
			using std::swap;
			if (m_idx == other.m_idx) {
				// swap(get<m_idx>(*this), tools::get<m_idx>(other));
				return [&]<size_t... Idxs>(std::index_sequence<Idxs...>) {
					(((m_idx == Idxs) ? (swap(get<Idxs>(*this), get<Idxs>(other)), 0) : 0), ...);
				}(std::make_index_sequence<sizeof...(Ts)>());
			} else {
				auto p = std::move(*this);
				std::construct_at(this, std::move(other));
				std::construct_at(&other, std::move(p));
			}
		}

		/**
		 * @brief Copy assignment operator.
		 */
		variant<Ts...> &operator=(const variant<Ts...> &other) {
			variant(other).swap(*this);
			return *this;
		}

		/**
		 * @brief Move assignment operator.
		 */
		variant<Ts...> &operator=(variant<Ts...> &&other) noexcept {
			variant(std::move(other)).swap(*this);
			return *this;
		}

		/**
		 * @brief Converting assignment operator. Assigns a value of
		 * type
		 * @p T to the variant.
		 */
		template <tools::exists<Ts...> T> constexpr variant<Ts...> &operator=(T &&element) {
			this->~variant();
			constexpr size_t i = tools::find_index_of_v<T, Ts...>;
			tools::construct_at<i>(m_data, TOOLS_FWD(element));
			m_idx = i;
			return *this;
		}

		/**
		 * @brief Destructor. Destroys the currently held alternative.
		 */
		constexpr ~variant() {
			constexpr auto idx_seq = std::make_index_sequence<sizeof...(Ts)>();
			[&]<auto... Indices>(std::index_sequence<Indices...>) {
				(((m_idx == Indices) ? (tools::destroy_at<Indices>(m_data), 0) : 0), ...);
			}(idx_seq);
		}

		/**
		 * @brief Returns the zero-based index of the alternative
		 * currently held by the variant.
		 */
		constexpr std::size_t index() const noexcept { return m_idx; }

		template <size_t I, typename... Us> constexpr friend decltype(auto) get(const variant<Us...> &v);

		template <typename T, typename... Us> constexpr friend T &get(const variant<Us...> &v);

		template <size_t I, typename... Us> constexpr friend decltype(auto) get(variant<Us...> &v);

		template <typename T, typename... Us> constexpr friend T &get(variant<Us...> &v);
	};

	template <size_t I, typename... Ts> constexpr decltype(auto) get(const variant<Ts...> &v) {
		return tools::get<I>(v.m_data);
	}

	template <typename T, typename... Ts> constexpr T &get(const variant<Ts...> &v) {
		return tools::get<tools::find_index_of_v<T, Ts...>>(v.m_data);
	}

	template <size_t I, typename... Ts> constexpr decltype(auto) get(variant<Ts...> &v) {
		return tools::get<I>(v.m_data);
	}

	template <typename T, typename... Ts> constexpr T &get(variant<Ts...> &v) {
		return tools::get<tools::find_index_of_v<T, Ts...>>(v.m_data);
	}
} // namespace xstd