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

			Legal Chunks:
			  istring
				reference-to-chunk
				list-of-chunks

		 */
		template <typename String>
		struct make_chunk {
			typedef typename boost::make_recursive_variant<
				String,
				std::list<boost::recursive_variant_>::iterator
				>::type type;
		};
	}

	template <typename T=wchar_t>
	class lstring {
		typedef std::basic_string<T> string_t;
		typedef typename detail::make_chunk<string_t>::type chunk_t;
	};

}

#endif//LOGICAL_STRING
