#ifndef CONSTRUCTION_TRANSACTION_H
#define CONSTRUCTION_TRANSACTION_H

#include <cstdint>
#include <exception>
#include <memory>
#include <ranges>
#include <tuple>
#include <utility>

namespace xstd::detail {
	template <typename T, typename Allocator> struct soa_split_buffer;

	/**
	 * @brief  ConstructionTransaction is an RAII wrapper used for book-keeping. If the client iteratively
	 * constructs a sequence of T object in the split_buffer, and if any of constructions throw an exception,
	 * ConstructionTransaction ensures that the split_buffer stays in a valid state.
	 * It provides basic exception guarantee, so no resources are leaked, and the split_buffer's invariants
	 * are intact.
	 *
	 * @tparam T
	 * @tparam Alloc
	 * @tparam Iterator
	 */
template <typename T, typename Alloc, typename Iterator> struct ConstructionTransaction {
		ConstructionTransaction(soa_split_buffer<T, Alloc> *parent, Iterator pos, Iterator end);

		ConstructionTransaction(soa_split_buffer<T, Alloc> *parent, Iterator pos, std::size_t n);

		~ConstructionTransaction();
		soa_split_buffer<T, Alloc> *parent_;
		Iterator pos_;
		const Iterator end_;

		friend class soa_split_buffer<T, Alloc>;
	};

	template <typename T, typename Alloc, typename Iterator>
	ConstructionTransaction<T, Alloc, Iterator>::ConstructionTransaction(soa_split_buffer<T, Alloc> *parent,
	                                                                     Iterator pos, Iterator end)
	    : parent_{parent}, pos_{pos}, end_{end} {
	}

	template <typename T, typename Alloc, typename Iterator>
	ConstructionTransaction<T, Alloc, Iterator>::ConstructionTransaction(soa_split_buffer<T, Alloc> *parent,
	                                                                     Iterator pos, std::size_t n)
	    : parent_{parent}, pos_{pos}, end_{pos + n} {
	}

	template <typename T, typename Alloc, typename Iterator>
	ConstructionTransaction<T, Alloc, Iterator>::~ConstructionTransaction() {
		parent_->set_sentinel(pos_);
	}

	/**
	 * @brief SOAVecElementConstructionTransaction is an RAII class that guarantees
	 * strong exception safety, when constructing an element in soa_split_buffer container.
	 *
	 * @tparam Allocator Custom Allocator
	 * @tparam Args Member types of T
	 */
	template <typename Allocator, typename... Args> struct SOAVecElementConstructionTransaction {
		std::tuple<Args &...> args_{};
		std::tuple<Args *...> ptrs_{};
		size_t num_elements_constructed{0};
		[[no_unique_address]] Allocator allocator_;

		SOAVecElementConstructionTransaction(Args... args, Args *...ptrs, Allocator allocator)
		    : args_{std::tie(args...)}, ptrs_{ptrs...}, allocator_{allocator} {}

		constexpr void destroy() {
			return [&]<size_t... Indices>(std::index_sequence<Indices...>){
				(((Indices < num_elements_constructed) ? (std::destroy_at(std::get<Indices>(ptrs_)), 0) : 0), ...);
			}(std::make_index_sequence<sizeof...(Args)>{});
		}

		template <typename Arg> constexpr void construct_impl(Arg &&arg, Arg *ptr) {
			std::construct_at(ptr, arg);
			++num_elements_constructed;
		}

		~SOAVecElementConstructionTransaction() {
			if (num_elements_constructed) {
				destroy();
			}
		}

		constexpr void construct() {
			template for (constexpr int I : std::views::iota(sizeof...(Args))) {
				construct_impl(std::get<I>(args_), std::get<I>(ptrs_));
			}
		};
	};
} // namespace xstd::detail
#endif
