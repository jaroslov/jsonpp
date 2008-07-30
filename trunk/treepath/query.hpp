#ifndef TREEPATH_QUERY
#define TREEPATH_QUERY

#include <boost/tuple/tuple.hpp>

namespace treepath {

	/*

		What is a Node according to the definition from XPath?

		                                      Parent
							                               |
								                             |
		First-Sibling--...--Previous-Sibling----Node----Next-Sibling--...--Last-Sibling
		                                        /  \
                                           /    \
                                 First-Child    Last-Child

		A Node also has a Tag (the "name" of the node). Finally, a Node knows _what_index_ of the query
		it is associated with.

		What is the minimal requirements of a Node that we need?
		  
		       get_children(Node) -> pair(child_iterator, child_iterator)

		Given this function we can _emulate_ the Parent, and the siblings for a given node.

	 */

	template <typename T, typename Tag>
	struct node_traits {
		typedef typename T::test_type test_type;
		typedef typename T::node_variant node_variant;
		typedef typename T::child_iterator child_iterator;
	};

	template <typename T, typename Tag>
	std::pair<typename node_traits<T, Tag>::child_iterator, typename node_traits<T, Tag>::child_iterator>
	get_children (T const& t, Tag) {
		return std::make_pair(t.begin(), t.end());
	}

	template <typename T, typename Tag>
	struct Node {
		typedef T node_type;
		typedef Tag tag_type;

		typedef node_traits<Node, Tag> node_traits_t;
		typedef typename node_traits_t::test_type test_type;
		typedef typename node_traits_t::node_variant node_variant;
		typedef typename node_traits_t::child_iterator child_iterator;

		Node (T const& nd, child_iterator fsib, child_iterator lsib, child_iterator s)
			: node(nd) {
			this->siblings.first = fsib;
			this->siblings.last = lsib;
			this->siblings.self = s;
			this->siblings.current = fsib;
			boost::tie(this->children.first, this->children.last) = get_children(this->node);
		}

		T const& node;
		test_type test;
		struct {
			child_iterator first, last;
		} children;
		struct {
			child_iterator first, last, self, current;
		} siblings;
	};

}

#endif//TREEPATH_QUERY
