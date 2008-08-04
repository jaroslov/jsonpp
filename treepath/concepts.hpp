#ifndef TREEPATH_CONCEPTS
#define TREEPATH_CONCEPTS

#include <utility/begin-end.hpp>
#include <boost/mpl/bool.hpp>

namespace treepath {

	template <typename Tag=void> struct treepath_ {};
	static const treepath_<> treepath = treepath_<>();

	// Nodes are expected to specialize the bel::iterator meta-function
	template <typename Node, typename Tag>
	struct node_traits {
		typedef Tag node_traits_tag;
		typedef typename Node::node_variant node_variant;
		typedef typename Node::node_test_type node_test_type;
	};

	template <typename Node, typename Tag>
	struct has_children : boost::mpl::false_ {};

	template <typename Node, typename Tag>
	struct has_attributes : boost::mpl::false_ {};

	template <typename Node, typename Tag>
	std::pair<typename bel::iterator<Node, treepath_<Tag> >::type,
						typename bel::iterator<Node, treepath_<Tag> >::type>
	children (Node const& node, treepath_<Tag> const& tag) {
		return bel::sequence(node, tag);
	}

	template <typename Node, typename Tag>
	typename node_traits<Node, Tag>::node_variant
	parent (Node const& node, Tag) {
		return node.get_parent();
	}

	template <typename Node, typename Tag>
	typename node_traits<Node, Tag>::node_test_type
	node_test (Node const& node, Tag) {
		return node.node_test();
	}

}

#endif//TREEPATH_CONCEPTS
