#ifndef TREEPATH_QUERY
#define TREEPATH_QUERY

#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <utility/regular_ptr.hpp>
#include <utility/begin-end.hpp>
#include <treepath/concepts.hpp>

namespace treepath {

	namespace detail {

		template <typename Variant, typename Test, typename Tag=treepath_<> >
		struct poly_node {
		};

		template <typename Node, typename Variant, typename Test, typename Tag=treepath_<> >
		struct node {
			typedef typename node_traits<Node, Tag>::child_iterator child_iterator;

			Node const* node_;
			struct {
				child_iterator first, last;
			} children;
			struct {
				child_iterator first, self, last;
			} siblings;
			Variant parent_;
			Test test_;
		};

	}

	template <typename Node, typename Path, typename Tag>
	void query (Node const& root, Path const& path, Tag const& tag) {
		typedef node_traits<Node, Tag> root_traits;
		typedef typename root_traits::node_variant node_variant;
		typedef typename root_traits::node_test_type node_test_type;

		typedef detail::poly_node<node_variant, node_test_type, Tag> poly_node;
		typedef boost::tuple<poly_node> work_item_type;
		typedef std::vector<work_item_type> work_list_type;

		work_list_type work_list;
	}

}

#endif//TREEPATH_QUERY
