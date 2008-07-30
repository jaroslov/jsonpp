#ifndef TREEPATH_SIMPLE_XPATH_PARSER
#define TREEPATH_SIMPLE_XPATH_PARSER

#include <vector>
#include <string>
#include <treepath/path.hpp>

namespace treepath {

	namespace simple_xpath {

		std::vector<std::string> split (std::string const& str, std::string const& delim) {
			std::vector<std::string> result;
			std::size_t offset = 0;
			while (true) {
				std::size_t pos = str.find_first_of(delim, offset);
				if (std::string::npos == pos)
					break;
				result.push_back(str.substr(offset, pos));
				offset = pos+delim.size();
			}
			result.push_back(str.substr(offset,str.size()));
			return result;
		}
		
		typedef treepath::path<std::string> path_t;
		
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
					throw std::exception();
				path_t::axis_type axis(treepath::name_enum::from_string(axis_parts[0]), axis_parts[1]);
				if ("*" == axis_parts[1])
					axis.test_m = true;
				path.path_m.push_back(axis);
			}
			return path;
		}
		
	}

}

#endif//TREEPATH_SIMPLE_XPATH_PARSER
