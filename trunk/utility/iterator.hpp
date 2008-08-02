#ifndef TRIVIAL_VARIANT_ITERATOR
#define TRIVIAL_VARIANT_ITERATOR

#include <boost/variant.hpp>
#include <utility/regular_ptr.hpp>

namespace utility {

	template <typename Value>
	struct trivial_value_iterator {
		typedef trivial_value_iterator<Value> tvi_type;
		
		typedef Value value_type;

		trivial_value_iterator () : maybe_value() {}
		template <typename Subtype>
		trivial_value_iterator (Subtype const& sub)	: maybe_value(sub) {}

		trivial_value_iterator (tvi_type const& other) {
			this->maybe_value = other.maybe_value;
		}

		tvi_type& operator = (tvi_type const& other) {
			this->maybe_value = other.maybe_value;
			return *this;
		}

		friend bool operator == (tvi_type const& left, tvi_type const& right) {
			if (left.maybe_value.empty() and right.maybe_value.empty())
				return true;
			if (left.maybe_value.empty() xor right.maybe_value.empty())
				return false;
			std::cout << "HERE" << *left.maybe_value << " " << *right.maybe_value << std::endl;
			return left.maybe_value == right.maybe_value;
		}
		friend bool operator != (tvi_type const& left, tvi_type const& right) {
			if (left.maybe_value.empty() and right.maybe_value.empty())
				return false;
			if (left.maybe_value.empty() xor right.maybe_value.empty())
				return true;
			return left.maybe_value != right.maybe_value;
		}

		tvi_type& operator ++ () {
			this->maybe_value.clear();
			return *this;
		}
		tvi_type operator ++ (int) {
			tvi_type other(*this);
			this->maybe_value.clear();
			return other;
		}

		Value& operator * () {
			return *this->maybe_value;
		}
		Value const& operator * () const {
			return *this->maybe_value;
		}

		regular_ptr<Value>& operator -> () {
			return this->maybe_value;
		}
		const regular_ptr<Value>& operator -> () const {
			return this->maybe_value;
		}

		regular_ptr<Value> maybe_value;
	};

}

#endif//TRIVIAL_VARIANT_ITERATOR

