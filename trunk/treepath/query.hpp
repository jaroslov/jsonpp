#ifndef TREEPATH_QUERY
#define TREEPATH_QUERY

#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <utility/regular_ptr.hpp>

namespace treepath {

	namespace detail {

		struct node_interface {
			virtual ~ node_interface () {}
		};

	}

	template <typename Node, typename Test, typename Predicate>
	void query (Node const& root, path<Test, Predicate> const& path) {
		typedef utility::regular_ptr<detail::node_interface> node_interface_t;
		typedef boost::tuple<node_interface_t, std::size_t> work_item_t;
		// get<0>(w_item) => node
		// get<1>(w_item) => path-index
		// get<2>(w_item) => iterator-base
		typedef std::vector<work_item_t> work_list_t;

		work_list_t work_queue;

		while (not work_queue.empty()) {
		}
	}

}

#endif//TREEPATH_QUERY
