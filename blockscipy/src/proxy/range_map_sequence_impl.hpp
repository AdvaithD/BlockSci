//
//  range_map_sequence_impl.hpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 9/23/18.
//


#ifndef proxy_range_map_sequence_impl_hpp
#define proxy_range_map_sequence_impl_hpp

#include "proxy_py.hpp"

#include <blocksci/chain/block.hpp>
#include <blocksci/scripts/script_variant.hpp>

#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>

template<ranges::category range_cat, typename R>
Proxy<Iterator<R>> mapSequence(ProxyIterator &seq, Proxy<any_view<R, range_cat>> &p2) {
	auto generic = seq.getGeneric();
	return std::function<Iterator<R>(std::any &)>{[=](std::any &val) -> Iterator<R> {
		return ranges::view::join(ranges::view::transform(generic(val), p2));
	}};
}

template <ranges::category range_cat, ranges::category in_cat>
void addProxyMapSequenceFuncsMethods(pybind11::class_<proxy_sequence<in_cat>> &cl) {
	using namespace blocksci;
	cl
	.def("_map", mapSequence<range_cat, Block>)
	.def("_map", mapSequence<range_cat, Transaction>)
	.def("_map", mapSequence<range_cat, Input>)
	.def("_map", mapSequence<range_cat, Output>)
	.def("_map", mapSequence<range_cat, AnyScript>)
	.def("_map", mapSequence<range_cat, EquivAddress>)

	.def("_map", mapSequence<range_cat, Cluster>)
	.def("_map", mapSequence<range_cat, TaggedCluster>)
	.def("_map", mapSequence<range_cat, TaggedAddress>)
	;
}

#endif /* proxy_range_map_optional_hpp */
