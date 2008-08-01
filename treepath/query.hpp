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

		template <typename Tag>
		struct get_first_child {
			typedef boost::any result_type;

			template <typename T>
			boost::any operator () (T const& reference) const {
				return this->stripped(*reference);
			}

			template <typename T>
			typename boost::enable_if<has_children<T, Tag>,	boost::any>::type
			stripped (T const& ref) const {
				std::cout << "GOT FIRST CHILD" << std::endl;
				return boost::any(children(ref).first);
			}

			template <typename T>
			typename boost::disable_if<has_children<T, Tag>,	boost::any>::type
			stripped (T const& ref) const {
				std::cout << "NO FIRST CHILD" << std::endl;
				return boost::any(0);
			}

			template <typename Variant>
			static boost::any go (Variant const& var) {
				get_first_child<Tag> gfc;
				return boost::apply_visitor(gfc, var);
			}
		};

		template <typename Variant, typename Tag>
		struct get_child_and_increment {
			typedef std::pair<bool, Variant> result_type;

			template <typename T>
			result_type operator () (T const& reference) const {
				return this->stripped(*reference);
			}

			template <typename T>
			typename boost::enable_if<has_children<T, Tag>,	result_type>::type
			stripped (T const& ref) const {
				typedef typename bel::iterator<T, Tag>::type child_iterator;
				child_iterator &iter = boost::any_cast<child_iterator>(*this->iterator);
				const child_iterator last = children(ref).second;
				std::cout << "Num left: " << (last - iter) << std::flush;
				if (last == iter)
					return std::make_pair(false, Variant());
				++iter;
				std::cout << " remainder: " << (last - iter) << std::endl;
				return std::make_pair(true, Variant(*iter));
			}

			template <typename T>
			typename boost::disable_if<has_children<T, Tag>, result_type>::type
			stripped (T const& ref) const {
				return std::make_pair(false, Variant());
			}

			static result_type go (Variant const& var, boost::any& iter) {
				get_child_and_increment<Variant, Tag> gcai;
				gcai.iterator = &iter;
				return boost::apply_visitor(gcai, var);
			}

			boost::any* iterator;
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

				std::wcout << L"Index: " << boost::get<index_>(top) << std::flush;
				std::wcout << L" Axis: " << axis << std::flush;
				std::wcout << L" Name: " << axis_name << std::endl;

				switch (axis_name) {
				case name_enum::self: this->handle_self(top, axis); break;
				case name_enum::child: this->handle_child(top, axis); break;
				default:
					std::wcout << L"Unknown " << std::endl;
					this->work_list.clear();
				};
			}
		}

		void handle_child (work_item_type& item, axis_type const& axis) {
			typedef detail::get_child_and_increment<node_variant, Tag> gcai;
			if (boost::get<iterator_>(item).empty()) {
				// there is no iterator, of any kind; get the first iterator
				boost::get<iterator_>(item) = detail::get_first_child<Tag>::go(*boost::get<node_>(item));
			}
			// get the next node, get a flag if the next node is valid
			bool end_of_sequence = false;
			boost::any result_iter;
			boost::tie(end_of_sequence, result_iter) = gcai::go(*boost::get<node_>(item), boost::get<iterator_>(item));
			this->work_list.clear();
		}

		void handle_self (work_item_type& item, axis_type const& axis) {
			if (this->satisfies_test(item, axis)) {
				boost::get<index_>(item) += 1;
				boost::get<alt_>(item) = name_enum::unknown;
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
