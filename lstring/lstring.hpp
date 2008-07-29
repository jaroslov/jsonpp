#ifndef LOGICAL_STRING
#define LOGICAL_STRING

// boost
#include <boost/variant.hpp>
#include <boost/variant/recursive_variant.hpp>
// STL
#include <string>
#include <list>

namespace lstring {

	namespace detail {
		/*
			We create a data-structure that looks (essentially) like a grammar table for a CFG.
			However, there are some extra rules for operating on the table.

		 */
		struct entry {
			
		};
		struct table {
		};
	}

	template <typename T=wchar_t>
	class lstring {
		typedef std::basic_string<T> string_t;
		typedef typename detail::make_chunk<string_t>::type chunk_t;
	};

}

#endif//LOGICAL_STRING
