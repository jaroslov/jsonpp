#ifndef TREEPATH_QUERY
#define TREEPATH_QUERY

#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <utility/regular_ptr.hpp>
#include <utility/begin-end.hpp>
#include <treepath/concepts.hpp>

namespace treepath {

	namespace detail {

	}

	template <typename Node, typename Predicate, typename Tag>
	void query (Node const& root, path<typename node_traits<Node, Tag>::node_test_type, Predicate> const& path) {
	}

}

#endif//TREEPATH_QUERY
