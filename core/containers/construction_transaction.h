#ifndef CONSTRUCTION_TRANSACTION_H
#define CONSTRUCTION_TRANSACTION_H

#include <cstdint>
#include <exception>
#include <memory>
#include <ranges>
#include <utility>

namespace xstd::detail {
	template <typename T, typename Allocator> struct soa_split_buffer;

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

	template <typename Allocator, typename... Args> struct SOAVecElementConstructionTransaction {
		template <typename... Pointers> static constexpr void destroy(Pointers *...pointers) {
			(std::destroy_at(pointers), ...);
		}

		template <typename Head, typename... Tail, typename... ArgsConstructed>
		static constexpr void construct_impl(Head &&head, Tail &&...tail, Head *d_first, Tail *...d_tail,
		                                     std::tuple<ArgsConstructed *...> &ptrs_constructed) {
			auto [... ptrs] = ptrs_constructed;
			try {
				std::construct_at(d_first, head);
				construct_impl(tail..., d_tail..., std::make_tuple(ptrs..., d_first));
			} catch (std::exception e) {
				destroy(ptrs...);
			}
		}

		static constexpr void construct(Args &&...args, Args *...dest) {
			construct_impl(args..., dest..., std::tuple{});
		};
	};
} // namespace xstd::detail
#endif
