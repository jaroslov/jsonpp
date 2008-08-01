#ifndef TREEPATH_PATH
#define TREEPATH_PATH

#include <iostream>
#include <string>
#include <stdexcept>

namespace treepath {

	struct predicate {
		typedef bool result_type;
		template <typename T>
		bool operator () (T const&) const { return false; }
		template <typename T1, typename T2>
		bool operator () (T1 const&, T2 const&) const { return false; }
	};

	struct bad_axis_name : public std::exception {
		bad_axis_name (std::string const& axis) : axis_("badly formed axis name: "+axis) {}
		virtual ~ bad_axis_name () throw() {}
		virtual const char* what () const throw() {
			return this->axis_.c_str();
		}
		std::string axis_;
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
			unknown = '0',
		};

		template <typename String>
		static name_e from_string (String const& Str) {
			const std::string str(Str.begin(), Str.end());
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
			else throw bad_axis_name(str);
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
			default: throw bad_axis_name("unknown enumeration");
			}
		}
	};

	struct bad_axis_nodetest : public std::exception {
		bad_axis_nodetest (std::string const& axis) : axis_("badly formed axis node-test: "+axis) {}
		virtual ~ bad_axis_nodetest () throw() {}
		virtual const char* what () const throw() {
			return this->axis_.c_str();
		}
		std::string axis_;
	};

	struct nodetest_enum {
		enum nodetest_e {
			node = 'n',
			nodewild = '*',
			text = 't',
			comment = 'c',
			processing_instruction = 'p',
			unknown = '0',
		};

		template <typename String>
		static nodetest_e from_string (String const& Str) {
			const std::string str(Str.begin(), Str.end());
			if ("*" == str) return nodewild;
			else if ("text()" == str) return text;
			else if ("comment()" == str) return comment;
			else if ("processing-instruction()" == str) return processing_instruction;
			else return node;
		}

		template <typename String>
		static const char* to_string (nodetest_e const& n, String const& node) {
			const std::string str(node.begin(), node.end());
			if (nodewild == n) return "*";
			else if (text == n) return "text()";
			else if (comment == n) return "comment()";
			else if (processing_instruction == n) return "processing-instruction()";
			else return str.c_str();
		}
	};

	template <typename Test, typename Predicate=predicate>
	struct axis {

		typedef name_enum::name_e name_t;
		typedef nodetest_enum::nodetest_e node_t;
		typedef Test test_t;
		typedef Predicate predicate_t;

		axis () {}
		axis (name_t const& nm, node_t const& nd, test_t const& tst, predicate_t const& pred=Predicate())
			: name(nm), node(nd), test(tst), predicate(pred) {}

		template <typename Char>
		friend std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& ostr, axis<Test, Predicate> const& a) {
			ostr << name_enum::to_string(a.name) << "::" << nodetest_enum::to_string(a.node, a.test);
			return ostr;
		}
		
		name_t name;
		node_t node;
		test_t test;
		predicate_t predicate;
	};

	template <typename Test, typename Predicate=predicate>
	struct path {
		typedef axis<Test, Predicate> axis_type;
		typedef std::vector<axis_type> path_type;
		
		axis_type const& operator [] (std::size_t i) const { return this->path_m[i]; }
		std::size_t size () const { return this->path_m.size(); }

		template <typename Char>
		friend std::basic_ostream<Char>& operator << (std::basic_ostream<Char>& ostr, path<Test, Predicate> const& p) {
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
