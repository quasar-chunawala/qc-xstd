#ifndef CONSTRUCTION_TRANSACTION_H
#define CONSTRUCTION_TRANSACTION_H

#include <cstdint>

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
} // namespace xstd::detail

#endif
