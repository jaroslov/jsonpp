#ifndef TREEPATH_PATH
#define TREEPATH_PATH

namespace treepath {

	struct predicate {
		typedef bool result_type;
		template <typename T>
		bool operator () (T const&) const { return false; }
		template <typename T1, typename T2>
		bool operator () (T1 const&, T2 const&) const { return false; }
	};
	
	struct name_enum {
		enum name_e {
			ancestor = 'a',
			ancestor_or_self = 'A',
			attribute = '@',
			child = 'c',
			descendant = 'd',
			descendant_or_self = 'D',
			following = 'f',
			following_sibling = 'F',
			namespace_ = 'n',
			parent = 'r',
			preceding = 'p',
			preceding_sibling = 'P',
			self = 's',
		};
		static name_e from_string (std::string const& str) {
			if ("ancestor" == str) return ancestor;
			else if ("ancestor-or-self" == str) return ancestor_or_self;
			else if ("attribute" == str) return attribute;
			else if ("child" == str) return child;
			else if ("descendant" == str) return descendant;
			else if ("descendant-or-self" == str) return descendant_or_self;
			else if ("following" == str) return following;
			else if ("following-sibling" == str) return following_sibling;
			else if ("namespace" == str) return namespace_;
			else if ("parent" == str) return parent;
			else if ("preceding" == str) return preceding;
			else if ("preceding-sibling" == str) return preceding_sibling;
			else if ("self" == str) return self;
			else throw std::exception();
		}
		static const char* to_string (name_e const& N) {
			switch (N) {
			case ancestor: return "ancestor";
			case ancestor_or_self: return "ancestor-or-self";
			case attribute: return "attribute";
			case child: return "child";
			case descendant: return "descendant";
			case descendant_or_self: return "descendant-or-self";
			case following: return "following";
			case following_sibling: return "following-sibling";
			case namespace_: return "namespace";
			case parent: return "parent";
			case preceding: return "preceding";
			case preceding_sibling: return "preceding-sibling";
			case self: return "self";
			default: throw std::exception();
			}
		}
	};

	template <typename Test, typename Predicate=predicate>
	struct axis {
		struct test_t {
			bool wild_m;
			Test test_m;
			test_t () {}
			test_t (bool b) : wild_m(b) {}
			test_t (Test const& t) : wild_m(false), test_m(t) {}
			void operator = (test_t const& t) {
				this->wild_m = t.wild_m;
				this->test_m = t.test_m;
			}
			void operator = (Test const& t) {
				this->wild_m = false;
				this->test_m = t;
			}
			void operator = (bool b) {
				this->wild_m = b;
			}
			operator bool () const {
				return this->wild_m;
			}
		};

		typedef name_enum::name_e name;
		typedef test_t test;
		typedef Predicate predicate;

		axis () {}
		axis (name const& N)
			: name_m(N) {}
		axis (name const& N, Test const& T)
			: name_m(N), test_m(T) {}
		axis (name const& N, Test const& T, predicate const& P)
			: name_m(N), test_m(T), predicate_m(P) {}

		friend std::ostream& operator << (std::ostream& ostr, axis<Test, Predicate> const& a) {
			ostr << name_enum::to_string(a.name_m) << "::";
			if (a.test_m)
				ostr << "*";
			else
				ostr << a.test_m.test_m;
			return ostr;
		}
		
		name name_m;
		test test_m;
		predicate predicate_m;
	};

	template <typename Test, typename Predicate=predicate>
	struct path {
		typedef axis<Test, Predicate> axis_type;
		typedef std::vector<axis_type> path_type;
		
		axis_type const& operator [] (std::size_t i) const { return this->path_m[i]; }
		std::size_t size () const { return this->path_m.size(); }

		friend std::ostream& operator << (std::ostream& ostr, path<Test, Predicate> const& p) {
			for (std::size_t i=0; i<p.size(); ++i) {
				ostr << p[i];
				if ((i+1) < p.size())
					ostr << "/";
			}
			return ostr;
		}

		path_type path_m;
	};
	
}

#endif//TREEPATH_PATH
