#ifndef TREEPATH_SIMPLE_XPATH_PARSER
#define TREEPATH_SIMPLE_XPATH_PARSER

#include <vector>
#include <string>
#include <treepath/path.hpp>
#include <stdexcept>

namespace treepath {

	namespace simple_xpath {

		std::vector<std::string> split (std::string const& str, std::string const& delim) {
			std::vector<std::string> result;
			std::size_t offset = 0;
			while (true) {
				std::size_t pos = str.find_first_of(delim, offset);
				if (std::string::npos == pos)
					break;
				result.push_back(str.substr(offset, pos-offset));
				offset = pos+delim.size();
			}
			result.push_back(str.substr(offset,str.size()));
			return result;
		}
		
		typedef treepath::path<std::string> path_t;

		struct bad_axis : public std::exception {
			bad_axis (std::string const& axis) : axis_("badly formed axis: "+axis) {}
			virtual ~ bad_axis () throw() {}
			virtual const char* what () const throw() {
				return this->axis_.c_str();
			}
			std::string axis_;
		};
		
		path_t parse (std::string const& str) {
			/*
				must be of this form:
				name::test/name::test/.../name::test
			*/
			path_t path;
			std::vector<std::string> path_parts = split(str, "/");
			for (std::size_t i=0; i<path_parts.size(); ++i) {
				std::vector<std::string> axis_parts = split(path_parts[i], "::");
				if (2 != axis_parts.size())
					throw bad_axis(path_parts[i]);
				path_t::axis_type axis(treepath::name_enum::from_string(axis_parts[0]),
															 treepath::nodetest_enum::from_string(axis_parts[1]),
															 axis_parts[1]);
				path.path_m.push_back(axis);
			}
			return path;
		}
		
	}

}

#endif//TREEPATH_SIMPLE_XPATH_PARSER
