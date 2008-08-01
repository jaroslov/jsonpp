#ifndef TREEPATH_QUERY
#define TREEPATH_QUERY

#include <vector>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/variant.hpp>
#include <utility/regular_ptr.hpp>
#include <utility/begin-end.hpp>
#include <treepath/concepts.hpp>

namespace treepath {

	namespace detail {

		template <typename Test, typename Tag>
		struct satisfies_test {
			typedef bool result_type;
			
			template <typename T>
			bool operator () (T const& reference) const {
				//std::wcout << L"Testing against: " << this->test << " == " << node_test(*reference, Tag()) << std::endl;;
				return this->test == node_test(*reference, Tag());
			}
			
			template <typename Variant>
			static bool go (Variant const& var, Test const& test) {
				satisfies_test<Test, Tag> st;
				st.test = test;
				return boost::apply_visitor(st, var);
			}
			
			Test test;
		};

	}

	template <typename Node, typename Path, typename Tag=treepath_<> >
	struct Query {
		typedef typename Path::axis_type axis_type;

		typedef node_traits<Node, Tag> root_traits;
		typedef typename root_traits::node_variant node_variant;
		typedef typename root_traits::node_test_type node_test_type;
		
		typedef boost::shared_ptr<node_variant> shared_node_variant;
		
		typedef boost::tuple<shared_node_variant, shared_node_variant, boost::any, std::size_t, name_enum::name_e> work_item_type;
		static const std::size_t node_ = 0;
		static const std::size_t parent_ = 1;
		static const std::size_t iterator_ = 2; // (either child or sibling)
		static const std::size_t index_ = 3; // for the path
		static const std::size_t alt_ = 4; // axis name
		
		typedef std::vector<work_item_type> work_list_type;
		
		void operator () (Node const& root, Path const& path) {
			this->path = &path;
			this->work_list.clear();
			this->snv_root = shared_node_variant(new node_variant(&root));
			
			this->work_list.push_back(boost::make_tuple(snv_root,
																									shared_node_variant(),
																									boost::any(),
																									0,
																									name_enum::unknown));
		}
		
		bool done () const {
			return this->work_list.empty();
		}
		
		void next () {
			// this is to get the next node, lazily
			// will do as much work as necessary to get the next node
			while (not this->work_list.empty()) {

				work_item_type &top = this->work_list.back();
				const std::size_t index = boost::get<index_>(top);

				if (this->path->size() <= index) {
					this->work_list.pop_back();
					return;
				}

				const axis_type &axis = (*this->path)[index];

				name_enum::name_e axis_name = axis.name;
				if (name_enum::unknown != boost::get<alt_>(top))
					axis_name = boost::get<alt_>(top);

				switch (axis_name) {
				case name_enum::self: this->handle_self(top, axis); break;
				default:
					this->work_list.clear();
				};
			}
		}			

		void handle_self (work_item_type& item, axis_type const& axis) {
			if (this->satisfies_test(item, axis)) {
				boost::get<index_>(item) += 1;
				boost::get<alt_>(item) = axis.name;
			} else
				this->work_list.pop_back();
		}
		
		bool satisfies_test (work_item_type& item, axis_type const& axis) {
			return detail::satisfies_test<node_test_type, Tag>::go(*boost::get<node_>(item), axis.test);
		}

		const Path *path;
		shared_node_variant snv_root;
		work_list_type work_list;
	};
	
	template <typename Node, typename Path, typename Tag>
	void query (Node const& root, Path const& path, Tag const& tag) {
		Query<Node, Path, Tag> Q;
		Q(root, path);
		while (not Q.done()) {
			Q.next();
		}
	}

}

#endif//TREEPATH_QUERY
