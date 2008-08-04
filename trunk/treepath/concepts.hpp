#ifndef TREEPATH_CONCEPTS
#define TREEPATH_CONCEPTS

#include <utility/begin-end.hpp>
#include <boost/mpl/bool.hpp>

namespace treepath {

	template <typename Tag=void> struct treepath_ {};

	template <typename Tag=void> struct children_ {};

	template <typename Tag=void> struct attributes_ {};

	// Nodes are expected to specialize the bel::iterator meta-function
	template <typename Node, typename Tag>
	struct node_traits {
		typedef Tag node_traits_tag;
		typedef typename Node::children_iterator_tag children_iterator_tag;
		typedef typename Node::attributes_iterator_tag attributes_iterator_tag;
		typedef typename Node::node_variant node_variant;
		typedef typename Node::node_test_type node_test_type;
	};

	template <typename Node, typename Tag>
	struct has_children : boost::mpl::false_ {};

	// a function returning a sequence-pair of iterators called `children': Node * children_iterator_tag => pair(iter, iter) must exist
	// the return type (iter) is bel::iterator<Node, children_iterator_tag>::type

	template <typename Node, typename Tag>
	struct has_attributes : boost::mpl::false_ {};

	// a function returning a sequence-pair of iterators called `attributes': Node * attributes_iterator_tag => pair(iter, iter) must exist
	// the return type (iter) is bel::iterator<Node, attributes_iterator_tag>::type

	template <typename Node, typename Tag>
	typename node_traits<Node, Tag>::node_variant
	parent (Node const& node, Tag) {
		return node.get_parent();
	}

	template <typename Node, typename Tag>
	typename node_traits<Node, Tag>::node_test_type
	test (Node const& node, Tag) {
		return node.node_test();
	}

	template <typename Node, typename Tag>
	bool is_node (Node const& node, Tag) {
		return true;
	}

	template <typename Node, typename Tag>
	bool is_text (Node const& node, Tag) {
		return false;
	}

	template <typename Node, typename Tag>
	bool is_processing_instruction (Node const& node, Tag) {
		return false;
	}

	template <typename Node, typename Tag>
	bool is_comment (Node const& node, Tag) {
		return false;
	}

}

#endif//TREEPATH_CONCEPTS
