#ifndef TREEPATH_SIMPLE_XPATH_PARSER
#define TREEPATH_SIMPLE_XPATH_PARSER

#include <vector>
#include <string>
#include <treepath/path.hpp>
#include <stdexcept>

namespace treepath {

	namespace simple_xpath {

		template <typename String>
		std::vector<String> split (String const& str, String const& delim) {
			std::vector<String> result;
			std::size_t offset = 0;
			while (true) {
				std::size_t pos = str.find_first_of(delim, offset);
				if (String::npos == pos)
					break;
				result.push_back(str.substr(offset, pos-offset));
				offset = pos+delim.size();
			}
			result.push_back(str.substr(offset,str.size()));
			return result;
		}
		
		template <typename String>
		struct bad_axis : public std::exception {
			bad_axis (String const& axis) : axis_("badly formed axis: "+axis) {}
			virtual ~ bad_axis () throw() {}
			virtual const char* what () const throw() {
				return this->axis_.c_str();
			}
			String axis_;
		};

		template <typename String>		
		path<String> parse (String const& str) {
			/*
				must be of this form:
				name::test/name::test/.../name::test
			*/
			path<String> path;
			std::vector<String> path_parts = split(str, "/");
			for (std::size_t i=0; i<path_parts.size(); ++i) {
				std::vector<String> axis_parts = split(path_parts[i], "::");
				if (2 != axis_parts.size())
					throw bad_axis<String>(path_parts[i]);
				typename path<String>::axis_type axis(treepath::name_enum::from_string(axis_parts[0]),
																							treepath::nodetest_enum::from_string(axis_parts[1]),
																							axis_parts[1]);
				path.path_m.push_back(axis);
			}
			return path;
		}
		
	}

}

#endif//TREEPATH_SIMPLE_XPATH_PARSER
