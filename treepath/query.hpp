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

			struct satisfies_test {
				typedef bool result_type;

				template <typename Node>
				result_type operator () (Node const& dereferenceable) const {
					return *this->test == node_test(*dereferenceable, tag_t());
				}

				static bool go (variant_t const& var, test_t const& test) {
					satisfies_test st;
					st.test = &test;
				}

				test_t const *test;
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

			void operator ++ () {
				while (not this->queue.empty()) {
					item_t &top = this->queue.back();

					if (this->path->size() <= boost::get<index>(top)) {
						variant_t const& var = *boost::get<node_ptr>(top);
						this->queue.pop_back();
						return;
					}

					axis_type const& axis = (*this->path)[boost::get<index>(top)];

					name_enum::name_e axis_name = axis.name;
					if (boost::get<alt_name>(top).first)
						axis_name = boost::get<alt_name>(top).second;

					switch (axis_name) {
					case name_enum::self: this->handle_self(top, axis); break;
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

			void handle_descendant (item_t& item, axis_type const& axis) {
				if (boost::get<iterator_any>(item).empty()) {
					std::wcout << L"Initialize first child for descendant" << std::endl;
					boost::get<alt_name>(item).first = true;
					boost::get<alt_name>(item).second = name_enum::unknown;
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
