#ifndef TREEPATH_QUERY
#define TREEPATH_QUERY

#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <utility/regular_ptr.hpp>
#include <utility/begin-end.hpp>

namespace treepath {

	namespace detail {

	}

	template <typename Node, typename Test, typename Predicate>
	void query (Node const& root, path<Test, Predicate> const& path) {
	}

}

#endif//TREEPATH_QUERY
