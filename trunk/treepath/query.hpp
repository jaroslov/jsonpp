#ifndef TREEPATH_QUERY
#define TREEPATH_QUERY

#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>

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

	struct treepath_ {};
	static const treepath_ treepath = treepath_();

	template <typename T, typename Tag=treepath_>
	struct node_traits {
		typedef typename T::test_type test_type;
		typedef typename T::node_variant node_variant;
		typedef typename T::child_iterator child_iterator;
	};

	template <typename T, typename Tag>
	std::pair<typename node_traits<T, Tag>::child_iterator, typename node_traits<T, Tag>::child_iterator>
	get_children (T const& t, Tag tag=treepath) {
		return std::make_pair(t.begin(), t.end());
	}

	template <typename Test, typename Tag=treepath_>
	struct NodeBase {
		typedef Test test_type;
		typedef Tag tag_type;

		virtual ~ NodeBase () {}
		virtual test_type const& test () const = 0;
		virtual void first_child () const = 0;
		virtual void last_child () const = 0;
		virtual void first_sibling () const = 0;
		virtual void last_sibling () const = 0;
		virtual void self_iterator () const = 0;

		boost::shared_ptr<NodeBase> parent;
	};

	template <typename T, typename Tag=treepath_>
	struct Node : public NodeBase<typename node_traits<T>::test_type, Tag> {
		typedef T node_type;
		typedef Tag tag_type;

		typedef node_traits<Node, Tag> node_traits_t;
		typedef typename node_traits_t::test_type test_type;
		typedef typename node_traits_t::node_variant node_variant;
		typedef typename node_traits_t::child_iterator child_iterator;

		Node () : node(0) {}

		Node (T const& nd, child_iterator fsib, child_iterator lsib, child_iterator s)
			: node(&nd) {
			this->siblings.first = fsib;
			this->siblings.last = lsib;
			this->siblings.self = s;
			boost::tie(this->children.first, this->children.last) = get_children(this->node);
		}

		virtual ~ Node () {
			this->node = 0;
		}

		virtual test_type const& test () const {
			return this->test_m;
		}
		virtual void first_child () const {}
		virtual void last_child () const {}
		virtual void first_sibling () const {}
		virtual void last_sibling () const {}
		virtual void self_iterator () const {}

		T const* node;
		test_type test_m;
		struct {
			child_iterator first, last;
		} children;
		struct {
			child_iterator first, last, self;
		} siblings;
	};

}

#endif//TREEPATH_QUERY
