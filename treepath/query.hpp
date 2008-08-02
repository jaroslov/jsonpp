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

			typedef boost::shared_ptr<variant_t> sh_variant_t;
			typedef std::pair<bool, name_enum::name_e> alternate_name;
			typedef boost::tuple<sh_variant_t, sh_variant_t, boost::any, std::size_t, alternate_name> item_t;
			typedef std::vector<item_t> work_queue_t;

			static const std::size_t node_ptr = 0;
			static const std::size_t parent_ptr = 1;
			static const std::size_t iterator_any = 2;
			static const std::size_t index = 3;
			static const std::size_t alt_name = 4;

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
			}

			operator bool () const {
				return false;
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
			break;
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
