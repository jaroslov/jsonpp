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
			typedef std::pair<bool, name_enum::name_e> alternate_name;
			typedef boost::tuple<sh_variant_t, sh_variant_t, boost::any, std::size_t, alternate_name> item_t;
			typedef std::vector<item_t> work_queue_t;

			static const std::size_t node_ptr = 0;
			static const std::size_t parent_ptr = 1;
			static const std::size_t iterator_any = 2;
			static const std::size_t index = 3;
			static const std::size_t alt_name = 4;

			struct no_child_iterator {};

			struct satisfies_test {
				typedef bool result_type;

				template <typename Node>
				result_type operator () (Node const& dereferenceable) const {
					return *this->test == node_test(*dereferenceable, tag_t());
				}

				static bool go (variant_t const& var, test_t const& test) {
					satisfies_test st;
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
					variant_t var = &**iter;
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

			struct try_get_str {
				typedef std::wstring result_type;

				template <typename Node>
				result_type operator () (Node const& dereferenceable) const {
					return this->try_get(*dereferenceable);
				}
				template <typename Not>
				result_type try_get (Not const& nope) const {
					std::string tyname = typeid(nope).name();
					return this->try_get(tyname);
				}
				result_type try_get (std::wstring const& wstr) const {
					return wstr;
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

				this->root = sh_variant_t(new variant_t(&root));
				this->queue.push_back(boost::make_tuple(this->root,
																								sh_variant_t(),
																								boost::any(),
																								0,
																								std::make_pair(false,
																															 name_enum::unknown)));
			}

			variant_t operator * () const {
				return this->result_value;
			}

			void operator ++ () {
				while (not this->queue.empty()) {
					item_t &top = this->queue.back();

					if (this->path->size() <= boost::get<index>(top)) {
						this->result_value = *boost::get<node_ptr>(top);
						this->queue.pop_back();
						return;
					}

					axis_type const& axis = (*this->path)[boost::get<index>(top)];

					name_enum::name_e axis_name = axis.name;
					if (boost::get<alt_name>(top).first)
						axis_name = boost::get<alt_name>(top).second;

					switch (axis_name) {
					case name_enum::self: this->handle_self(top, axis); break;
					case name_enum::child: this->handle_child(top, axis); break;
					case name_enum::descendant: this->handle_descendant(top, axis); break;
					case name_enum::descendant_or_self: this->handle_descendant_or_self(top, axis); break;
					default:
						this->queue.clear();
						std::wcout << L"Did not recognize: " << axis << L" as " << name_enum::to_string(axis_name) << std::endl;
					}
				}
			}

			bool test (item_t& item, axis_type const& axis) {
				return (nodetest_enum::wildcard == axis.node)
					or satisfies_test::go(*boost::get<node_ptr>(item), axis.test);
			}

			void handle_self (item_t& item, axis_type const& axis) {
				if (this->test(item, axis)) {
					++boost::get<index>(item);
				} else
					this->queue.pop_back();
			}

			void handle_child (item_t& item, axis_type const& axis) {
				// add children as 'self'
				if (boost::get<iterator_any>(item).empty()) {
					boost::get<iterator_any>(item) = get_first_child::go(*boost::get<node_ptr>(item));
				}
				bool valid_item = false;
				variant_t child_item;
				boost::tie(valid_item, child_item) = get_child_from::go(*boost::get<node_ptr>(item), boost::get<iterator_any>(item));
				if (valid_item) {
					std::wcout << L"Add Child: " << try_get_str::go(child_item) << std::endl;
					item_t child = item;
					boost::get<node_ptr>(child) = sh_variant_t(new variant_t(child_item));
					boost::get<parent_ptr>(child) = boost::get<node_ptr>(item); // item is the child's parent
					boost::get<alt_name>(child).first = true;
					boost::get<alt_name>(child).second = name_enum::self;
					this->queue.push_back(child);
				} else {
					this->queue.pop_back();
				}
			}

			void handle_descendant (item_t& item, axis_type const& axis) {
				//// we add a 'descendant' entry for each child
				//   we add a 'child' entry for ourselves, once
				//
				// a call to descendant:
				//   1. if the iterator is "empty", this is the entry point, get the first iterator
				bool add_self_as_child = false;
				if (boost::get<iterator_any>(item).empty()) {
					add_self_as_child = true;
					boost::get<iterator_any>(item) = get_first_child::go(*boost::get<node_ptr>(item));
				}
				//   2. we have an iterator, dereference it & increment it
				bool valid_item = false; // false if we're at the end of the sequence
				variant_t child_item; // the variant reference
				boost::tie(valid_item, child_item) = get_child_from::go(*boost::get<node_ptr>(item), boost::get<iterator_any>(item));
				if (valid_item) {
					std::wcout << L"Add Descendant" << std::endl;
					// 3. we have a valid child
					item_t descendant = item;
					boost::get<alt_name>(descendant) = std::make_pair(true, name_enum::descendant);
					boost::get<node_ptr>(descendant) = sh_variant_t(new variant_t(child_item));
					boost::get<parent_ptr>(descendant) = boost::get<parent_ptr>(item);
					this->queue.push_back(descendant);
				} else {
					// 4. out of children... so pop item
					this->queue.pop_back();
				}
				if (add_self_as_child) {
					item_t child = item;
					boost::get<iterator_any>(child) = boost::any();
					boost::get<alt_name>(child).first = true;
					boost::get<alt_name>(child).second = name_enum::child;
					this->queue.push_back(child);
				}
			}

			void handle_descendant_or_self (item_t& item, axis_type const& axis) {
				item_t self = item;
				boost::get<alt_name>(self) = std::make_pair(true, name_enum::self);
				item_t descendant = item;
				boost::get<alt_name>(descendant) = std::make_pair(true, name_enum::descendant);
				this->queue.pop_back();
				this->queue.push_back(descendant);
				this->queue.push_back(self);
			}

			operator bool () const {
				return this->queue.empty();
			}

			work_queue_t queue;
			Path const* path;
			sh_variant_t root;
			variant_t result_value;
		};

	}
	
	template <typename Node, typename Path, typename Tag>
	void query (Node const& root, Path const& path, Tag const& tag) {
		query_with_traits(node_traits<Node, Tag>(), root, path);
	}

	template <typename Traits, typename Node, typename Path>
	void query_with_traits (Traits, Node const& node, Path const& path) {
		detail::query<Traits, Path> query;
		query.initialize(node, path);
		while (not query) {
			++query;
			if (query)
				std::wcout << detail::query<Traits, Path>::try_get_str::go(*query) << std::endl;
		}
	}

	struct unwrap_ {};
	static const unwrap_ unwrap = unwrap_();

	template <typename Node, typename Path, typename Tag>
	struct unwrap_query {
		typedef void result_type;

		unwrap_query (Path const& p) : path(p) {}

		template <typename T>
		result_type operator () (T const& node) const {
			query_with_traits(node_traits<Node, Tag>(), node, this->path);
		}

		static result_type go (Node const& node, Path const& path, Tag) {
			unwrap_query<Node, Path, Tag> uq(path);
			boost::apply_visitor(uq, node);
		}

		Path const& path;
	};

	template <typename Node, typename Path, typename Tag>
	void query (Node const& root, Path const& path, Tag const& tag, unwrap_) {
		unwrap_query<Node, Path, Tag>::go(root, path, tag);
	}

}

#endif//TREEPATH_QUERY
