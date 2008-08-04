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
			typedef typename node_traits::children_iterator_tag child_tag_t;
			typedef typename node_traits::attributes_iterator_tag attr_tag_t;
			typedef typename node_traits::node_test_type test_t;
			typedef typename node_traits::node_variant variant_t;

			typedef typename Path::location_type location_type;

			typedef boost::shared_ptr<variant_t> sh_variant_t;
			typedef std::pair<bool, axis_enum::axis_e> alternate_axis_t;

			struct item_t {
				typedef boost::shared_ptr<item_t> sh_item_t;

				item_t () : alternate_axis(false, axis_enum::unknown) {}
				item_t (variant_t const& node, std::size_t idx)
					: node(node), index(idx)
					, alternate_axis(false, axis_enum::unknown) {}
				item_t (variant_t const& node, axis_enum::axis_e const& axis)
					: node(node), alternate_axis(true, axis) {}

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
				alternate_axis_t alternate_axis;
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
					return boost::any(children(node, child_tag_t()).first);
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
					typedef typename bel::iterator<Node, child_tag_t>::type child_iterator;
					child_iterator *iter = boost::any_cast<child_iterator>(this->iterator);
					const child_iterator last = children(node, child_tag_t()).second;
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

			struct get_first_attribute {
				typedef boost::any result_type;

				template <typename Node>
				result_type operator () (Node const& dereferenceable) const {
					return this->get_attr(*dereferenceable);
				}

				template <typename Node>
				typename boost::enable_if<has_attributes<Node, tag_t>, boost::any>::type
				get_attr (Node const& node) const {
					return boost::any(attributes(node, attr_tag_t()).first);
				}
				template <typename Node>
				typename boost::disable_if<has_attributes<Node, tag_t>, boost::any>::type
				get_attr (Node const& node) const {
					return boost::any();
				}

				static boost::any go (variant_t const& var) {
					get_first_attribute gfa;
					return boost::apply_visitor(gfa, var);
				}
			};

			struct get_attribute_from {
				typedef std::pair<bool, variant_t> result_type;

				template <typename Node>
				result_type operator () (Node const& dereferenceable) const {
					return this->get_attr(*dereferenceable);
				}

				template <typename Node>
				typename boost::enable_if<has_attributes<Node, tag_t>, result_type>::type
				get_attr (Node const& node) const {
					typedef typename bel::iterator<Node, attr_tag_t>::type attr_iterator;
					attr_iterator *iter = boost::any_cast<attr_iterator>(this->iterator);
					const attr_iterator last = attributes(node, attr_tag_t()).second;
					if (last == *iter)
						return result_type(false, variant_t());
					variant_t var = get_reference(**iter, tag_t());
					++*iter;
					return result_type(true, var);
				}
				template <typename Node>
				typename boost::disable_if<has_attributes<Node, tag_t>, result_type>::type
				get_attr (Node const& node) const {
					return result_type(false, variant_t());
				}

				static result_type go (variant_t const& var, boost::any& iter) {
					get_attribute_from gaf;
					gaf.iterator = &iter;
					return boost::apply_visitor(gaf, var);
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

					location_type const& location = (*this->path)[top->index];

					axis_enum::axis_e axis_name = location.axis;
					if (top->alternate_axis.first)
						axis_name = top->alternate_axis.second;

					switch (axis_name) {
					case axis_enum::ancestor: this->handle_ancestor(top, location); break;
					case axis_enum::ancestor_or_self: this->handle_ancestor_or_self(top, location); break;
					case axis_enum::attribute: this->handle_attribute(top, location); break;
					case axis_enum::child: this->handle_child(top, location); break;
					case axis_enum::descendant: this->handle_descendant(top, location); break;
					case axis_enum::descendant_or_self: this->handle_descendant_or_self(top, location); break;
						//case axis_enum::following: this->handle_following(top, location); break;
						//case axis_enum::following_sibling: this->handle_following_sibling(top, location); break;
						//case axis_enum::namespace_: this->handle_namespace(top, location); break;
					case axis_enum::parent: this->handle_parent(top, location); break;
						//case axis_enum::preceding: this->handle_preceding(top, location); break;
						//case axis_enum::preceding_sibling: this->handle_preceding_sibling(top, location); break;
					case axis_enum::self: this->handle_self(top, location); break;
					default:
						this->queue.clear();
						std::wcout << L"Did not recognize: " << location << L" as " << axis_enum::to_string(axis_name) << std::endl;
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

			bool test_node (item_t const& item, location_type const& location) {
				bool node_test_p = satisfies_node_test::go(item.node, location.test);
				switch (location.node) {
				case nodetest_enum::node:	return is_node(item.node, tag_t());
				case nodetest_enum::text: return is_text(item.node, tag_t());
				case nodetest_enum::comment: return is_comment(item.node, tag_t());
				case nodetest_enum::processing_instruction: return is_processing_instruction(item.node, tag_t()) and node_test_p;
				default: return node_test_p;
				}
			}

			void handle_self (sh_item_t& item, location_type const& location) {
				// a terminal; a call to self resolves to an index increase or a pop
				if (this->test_node(*item, location)) {
					++item->index;
					item->alternate_axis.first = false;
				} else
					this->queue.pop_back();
			}

			void handle_parent (sh_item_t& item, location_type const&) {
				// there is only ever (at most) one parent, so we always pop it
				sh_item_t current = item;
				this->queue.pop_back();
				if (current->parent_ptr) {
					sh_item_t parent = sh_item_t(new item_t(current->parent_ptr->node, current->index));
					parent->clear_state();
					parent->alternate_axis.first = true;
					parent->alternate_axis.second = axis_enum::self;
					parent->parent_ptr = current->parent_ptr->parent_ptr;
					parent->sibling.self = current->parent_ptr->sibling.self;
					this->queue.push_back(parent);
				}
			}

			void handle_ancestor (sh_item_t& item, location_type const&) {
				// if there is a parent, add self as an ancestor and then add self as parent
				sh_item_t current = item;
				this->queue.pop_back();
				if (current->parent_ptr) {
					sh_item_t parent = sh_item_t(new item_t(current->parent_ptr->node, current->index));
					parent->alternate_axis.first = true;
					parent->alternate_axis.second = axis_enum::ancestor;
					parent->parent_ptr = current->parent_ptr->parent_ptr;
					this->queue.push_back(parent);
					item->alternate_axis.first = true;
					item->alternate_axis.second = axis_enum::parent;
					this->queue.push_back(item);
				}
			}

			void handle_ancestor_or_self (sh_item_t& item, location_type const&) {
				sh_item_t self = sh_item_t(new item_t(item->node, item->index));
				self->parent_ptr = item->parent_ptr;
				self->sibling.self = item->sibling.self;
				self->alternate_axis.first = true;
				self->alternate_axis.second = axis_enum::self;
				item->alternate_axis.first = true;
				item->alternate_axis.second = axis_enum::ancestor;
				this->queue.push_back(self);
			}

			template <typename GetFirst, typename GetFrom>
			void handle_subnode (sh_item_t& item, location_type const&, boost::any& iter, GetFirst, GetFrom) {
				if (iter.empty()) {
					iter = GetFirst::go(item->node);
				}

				bool valid_item = false;
				variant_t subnode_var;
				boost::tie(valid_item, subnode_var) = GetFrom::go(item->node, iter);

				if (valid_item) {
					sh_item_t subnode = sh_item_t(new item_t(subnode_var, item->index));
					subnode->parent_ptr = item;
					subnode->alternate_axis = std::make_pair(true, axis_enum::self);
					subnode->sibling.self = iter;
					this->queue.push_back(subnode);
				} else
					this->queue.pop_back();
			}

			void handle_child (sh_item_t& item, location_type const& loc) {
				this->handle_subnode(item, loc, item->child_iterator, get_first_child(), get_child_from());
			}

			void handle_attribute (sh_item_t& item, location_type const& loc) {
				this->handle_subnode(item, loc, item->attribute_iterator, get_first_attribute(), get_attribute_from());
			}
	
			void handle_descendant (sh_item_t& item, location_type const&) {
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
					descendant->alternate_axis.first = true;
					descendant->alternate_axis.second = axis_enum::descendant;
					descendant->sibling.self = item->child_iterator;
					this->queue.push_back(descendant);
				} else
					this->queue.pop_back();

				if (add_self_as_child) {
					sh_item_t child = sh_item_t(new item_t(*item));
					child->clear_state();
					child->parent_ptr = item->parent_ptr;
					child->sibling.self = item->sibling.self;
					child->alternate_axis.first = true;
					child->alternate_axis.second = axis_enum::child;
					this->queue.push_back(child);
				}
			}

			void handle_descendant_or_self (sh_item_t& item, location_type const&) {
				item->alternate_axis.first = true;
				item->alternate_axis.second = axis_enum::descendant;
				sh_item_t self = sh_item_t(new item_t(*item));
				self->alternate_axis.second = axis_enum::self;
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
