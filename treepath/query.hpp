#ifndef TREEPATH_QUERY
#define TREEPATH_QUERY

#include <vector>

#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/variant.hpp>
#include <boost/utility.hpp>

#include <utility/regular_ptr.hpp>
#include <utility/begin-end.hpp>

#include <treepath/concepts.hpp>

namespace treepath {

	namespace detail {

		template <typename Traits, typename Path>
		struct query {
			typedef Traits node_traits;
			typedef typename node_traits::node_traits_tag tag_t;
			typedef typename node_traits::node_test_type test_t;
			typedef typename node_traits::node_variant variant_t;

			typedef typename Path::axis_type axis_type;

			typedef boost::shared_ptr<variant_t> sh_variant_t;
			typedef std::pair<bool, name_enum::name_e> alternate_name_t;

			struct item_t {
				typedef boost::shared_ptr<item_t> sh_item_t;

				item_t () : alternate_name(false, name_enum::unknown) {}
				item_t (variant_t const& node, std::size_t idx)
					: node(node), index(idx)
					, alternate_name(false, name_enum::unknown) {}
				item_t (variant_t const& node, name_enum::name_e const& name)
					: node(node), alternate_name(true, name) {}

				void clear_state () {
					// clears the child_iterator & sibling.iterator; this is the "state",
					// as opposed to the "values"
					this->child_iterator = boost::any();
					this->sibling.iterator = boost::any();
					this->attribute_iterator = boost::any();
				}

				variant_t node;
				sh_item_t parent_ptr;
				std::size_t index;
				alternate_name_t alternate_name;
				boost::any child_iterator, attribute_iterator;
				struct {
					boost::any iterator;
					boost::any self;
				} sibling;
			};
			typedef boost::shared_ptr<item_t> sh_item_t;
			typedef std::vector<sh_item_t> work_queue_t;

			struct satisfies_node_test {
				typedef bool result_type;

				template <typename Node>
				result_type operator () (Node const& dereferenceable) const {
					return *this->test == node_test(*dereferenceable, tag_t());
				}

				static bool go (variant_t const& var, test_t const& test) {
					satisfies_node_test st;
					st.test = &test;
					return boost::apply_visitor(st, var);
				}

				test_t const *test;
			};

			struct get_first_child {
				typedef boost::any result_type;

				template <typename Node>
				result_type operator () (Node const& dereferenceable) const {
					return this->get_child(*dereferenceable);
				}

				template <typename Node>
				typename boost::enable_if<has_children<Node, tag_t>, boost::any>::type
				get_child (Node const& node) const {
					return boost::any(children(node, tag_t()).first);
				}
				template <typename Node>
				typename boost::disable_if<has_children<Node, tag_t>, boost::any>::type
				get_child (Node const& node) const {
					return boost::any();
				}

				static boost::any go (variant_t const& var) {
					get_first_child gfc;
					return boost::apply_visitor(gfc, var);
				}
			};

			struct get_child_from {
				typedef std::pair<bool, variant_t> result_type;

				template <typename Node>
				result_type operator () (Node const& dereferenceable) const {
					return this->get_child(*dereferenceable);
				}

				// get-child...
				template <typename Node>
				typename boost::enable_if<has_children<Node, tag_t>, result_type>::type
				get_child (Node const& node) const {
					typedef typename bel::iterator<Node, tag_t>::type child_iterator;
					child_iterator *iter = boost::any_cast<child_iterator>(this->iterator);
					const child_iterator last = children(node, tag_t()).second;
					if (last == *iter)
						return result_type(false, variant_t());
					variant_t var = get_reference(**iter, tag_t());
					++*iter;
					return result_type(true, var);
				}
				template <typename Node>
				typename boost::disable_if<has_children<Node, tag_t>, result_type>::type
				get_child (Node const& node) const {
					return result_type(false, variant_t());
				}

				static result_type go (variant_t const& var, boost::any& iter) {
					get_child_from gcf;
					gcf.iterator = &iter;
					return boost::apply_visitor(gcf, var);
				}

				boost::any *iterator;
			};

			struct get_node_test {
				typedef test_t result_type;

				template <typename T>
				result_type operator () (T const& t) const {
					return node_test(*t, tag_t());
				}

				static result_type go (variant_t const& var) {
					get_node_test rt;
					return boost::apply_visitor(rt, var);
				}
			};

			struct try_get_str {
				typedef std::wstring result_type;

				template <typename Node>
				result_type operator () (Node const& dereferenceable) const {
					return this->try_get(*dereferenceable);
				}
				template <typename Not>
				result_type try_get (Not const& nope) const {
					std::string tyname = typeid(nope).name();
					//return this->try_get(tyname);
					return L"(-?-)";
				}
				result_type try_get (std::wstring const& wstr) const {
					return L"\""+wstr+L"\"";
				}
				result_type try_get (std::string const& str) const {
					return std::wstring(str.begin(), str.end());
				}

				static result_type go (variant_t const& var) {
					try_get_str tgs;
					return boost::apply_visitor(tgs, var);
				}
			};

			query () {}

			template <typename Node>
			void initialize (Node const& root, Path const& path) {
				this->path = &path;
				this->queue.clear();

				this->root = sh_item_t(new item_t(get_reference(root, tag_t()),0));
				this->queue.push_back(this->root);
				this->get_next_valid();
			}

			variant_t operator * () const {
				return this->result_value;
			}

			void operator ++ () {
				this->get_next_valid();
			}

			void get_next_valid () {
				while (not this->queue.empty()) {
					sh_item_t &top = this->queue.back();

					if (this->path->size() <= top->index) {
						this->result_value = top->node;
						this->list_ancestors(top);
						this->queue.pop_back();
						return;
					}

					axis_type const& axis = (*this->path)[top->index];

					name_enum::name_e axis_name = axis.name;
					if (top->alternate_name.first)
						axis_name = top->alternate_name.second;

					switch (axis_name) {
						//case name_enum::attribute: this->handle_attribute(top, axis); break;
					case name_enum::ancestor: this->handle_ancestor(top, axis); break;
					case name_enum::ancestor_or_self: this->handle_ancestor_or_self(top, axis); break;
					case name_enum::child: this->handle_child(top, axis); break;
					case name_enum::descendant: this->handle_descendant(top, axis); break;
					case name_enum::descendant_or_self: this->handle_descendant_or_self(top, axis); break;
					case name_enum::parent: this->handle_parent(top, axis); break;
					case name_enum::self: this->handle_self(top, axis); break;
					default:
						this->queue.clear();
						std::wcout << L"Did not recognize: " << axis << L" as " << name_enum::to_string(axis_name) << std::endl;
					}
				}
			}

			void list_ancestors (sh_item_t const& shitem, bool non_self=false) {
				std::wcout << (non_self?L" => ":L"Leaf: ") << get_node_test::go(shitem->node) << std::flush;
				if (shitem->parent_ptr)
					this->list_ancestors(shitem->parent_ptr, true);
				if (not non_self)
					std::wcout << std::endl;
			}

			bool test_node (item_t const& item, axis_type const& axis) {
				return (nodetest_enum::wildcard == axis.node)
					or satisfies_node_test::go(item.node, axis.test);
			}

			void handle_self (sh_item_t& item, axis_type const& axis) {
				// a terminal; a call to self resolves to an index increase or a pop
				if (this->test_node(*item, axis)) {
					++item->index;
					item->alternate_name.first = false;
				} else
					this->queue.pop_back();
			}

			void handle_parent (sh_item_t& item, axis_type const& axis) {
				// there is only ever (at most) one parent, so we always pop it
				sh_item_t current = item;
				this->queue.pop_back();
				if (current->parent_ptr) {
					sh_item_t parent = sh_item_t(new item_t(current->parent_ptr->node, current->index));
					parent->alternate_name.first = true;
					parent->alternate_name.second = name_enum::self;
					if (current->parent_ptr->parent_ptr) // add the ancestors, as necessary
						parent->parent_ptr = current->parent_ptr->parent_ptr;
					this->queue.push_back(parent);
				}
			}

			void handle_ancestor (sh_item_t& item, axis_type const& axis) {
				// if there is a parent, add self as an ancestor and then add self as parent
				sh_item_t current = item;
				this->queue.pop_back();
				if (current->parent_ptr) {
					sh_item_t parent = sh_item_t(new item_t(current->parent_ptr->node, current->index));
					parent->alternate_name.first = true;
					parent->alternate_name.second = name_enum::ancestor;
					if (current->parent_ptr->parent_ptr) // add the ancestors, as necessary
						parent->parent_ptr = current->parent_ptr->parent_ptr;
					this->queue.push_back(parent);
					item->alternate_name.first = true;
					item->alternate_name.second = name_enum::parent;
					this->queue.push_back(item);
				}
			}

			void handle_ancestor_or_self (sh_item_t& item, axis_type const& axis) {
				throw std::exception();
			}

			void handle_attribute (sh_item_t& item, axis_type const& axis) {
				//// We build a new sh-item; if it works, then we add the attribute
				//// otherwise we pop
				throw std::exception();
			}

			void handle_child (sh_item_t& item, axis_type const& axis) {
				// add children as 'self'
				if (item->child_iterator.empty()) {
					item->child_iterator = get_first_child::go(item->node);
				}

				bool valid_item = false;
				variant_t child_var;
				boost::tie(valid_item, child_var) = get_child_from::go(item->node, item->child_iterator);

				if (valid_item) {
					sh_item_t child = sh_item_t(new item_t(child_var, item->index));
					child->parent_ptr = item;
					child->alternate_name.first = true;
					child->alternate_name.second = name_enum::self;
					child->sibling.self = item->child_iterator;
					this->queue.push_back(child);
				} else {
					this->queue.pop_back();
				}
			}
	
			void handle_descendant (sh_item_t& item, axis_type const& axis) {
				//// we add a 'descendant' entry for each child
				//   we add a 'child' entry for ourselves, once
				//
				// a call to descendant:
				//   1. if the iterator is "empty", this is the entry point, get the first iterator
				bool add_self_as_child = false;
				if (item->child_iterator.empty()) {
					add_self_as_child = true;
					item->child_iterator = get_first_child::go(item->node);
					if (item->child_iterator.empty()) {
						// the item has no children; so give up
						this->queue.pop_back();
						return;
					}
				}

				bool valid_item = false;
				variant_t child_var;
				boost::tie(valid_item, child_var) = get_child_from::go(item->node, item->child_iterator);
				if (valid_item) {
					sh_item_t descendant = sh_item_t(new item_t(child_var, item->index));
					descendant->clear_state();
					descendant->parent_ptr = item;
					descendant->alternate_name.first = true;
					descendant->alternate_name.second = name_enum::descendant;
					descendant->sibling.self = item->child_iterator;
					this->queue.push_back(descendant);
				} else
					this->queue.pop_back();

				if (add_self_as_child) {
					sh_item_t child = sh_item_t(new item_t(*item));
					child->clear_state();
					child->alternate_name.first = true;
					child->alternate_name.second = name_enum::child;
					this->queue.push_back(child);
				}
			}

			void handle_descendant_or_self (sh_item_t& item, axis_type const& axis) {
				item->alternate_name.first = true;
				item->alternate_name.second = name_enum::descendant;
				sh_item_t self = sh_item_t(new item_t(*item));
				self->alternate_name.second = name_enum::self;
				self->parent_ptr = item->parent_ptr;
				self->sibling.self = item->sibling.self;
				self->clear_state();
				this->queue.push_back(self);
			}

			operator bool () const {
				return this->queue.empty();
			}

			work_queue_t queue;
			Path const* path;
			sh_item_t root;
			variant_t result_value;
		};

	}
	
	template <typename Node, typename Path, typename Tag>
	std::vector<typename node_traits<Node, Tag>::node_variant>
	query (Node const& root, Path const& path, Tag const& tag) {
		return query_with_traits(node_traits<Node, Tag>(), root, path);
	}

	template <typename Traits, typename Node, typename Path>
	std::vector<typename Traits::node_variant>
	query_with_traits (Traits, Node const& node, Path const& path) {
		std::vector<typename Traits::node_variant> result;
		detail::query<Traits, Path> query;
		query.initialize(node, path);
		while (not query) {
			result.push_back(*query);
			++query;
		}
		return result;
	}

}

#endif//TREEPATH_QUERY
