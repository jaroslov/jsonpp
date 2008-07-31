#ifndef VALUED_INTERFACE_POINTER
#define VALUED_INTERFACE_POINTER

/*

A very simplistic implementation of a valued-interface pointer. The idea is
that we want a value-semantic (deep-copy) pointer for some type V, where we
only want to interface to V via some base-class U. This allows us to store
a heterogeneous set of values of types V_1,...,V_k as value-semantic
smart pointers with type U.

There should also be a weak-pointer version of this, as well.

*/

namespace smart_ptr {
	
	namespace detail {

		template <typename T>
		struct regular_storage {
			virtual ~ regular_storage () {}
			virtual T* get_ptr () = 0;
			virtual const T* get_ptr() const = 0;
			virtual regular_storage<T>* copy () const = 0;
			virtual bool equals (regular_storage<T> const& rs) const = 0;
			virtual bool not_equals (regular_storage<T> const& rs) const = 0;
		};

		template <typename T, typename U>
		struct cloning_storage : regular_storage<U> {
			cloning_storage () : value_() {}
			cloning_storage (T const& t) : value_(t) {}
			virtual ~ cloning_storage () {}
			virtual U* get_ptr () { return &this->value_; }
			virtual const U* get_ptr () const { return &this->value_; }
			virtual regular_storage<U>* copy () const {
				return new cloning_storage<T, U>(this->value_);
			}
			virtual bool equals (regular_storage<U> const* rs) const {
				const T *downcast_other = dynamic_cast<const T*>(rs->get_ptr());
				return downcast_other and (*downcast_other == this->value_);
			}
			virtual bool not_equals (regular_storage<U> const* rs) const {
				const T *downcast_other = dynamic_cast<const T*>(rs->get_ptr());
				if (not downcast_other) return true;
				return *downcast_other != this->value_;
			}

			T value_;
		};

	}

	template <typename T>
	class regular_ptr {
	public:
		regular_ptr () : indirect_value_(0) {}
		regular_ptr (regular_ptr<T> const& r) {
			this->indirect_value_ = 0;
			this->copy_from(r);
		}
		template <typename U>
		regular_ptr (regular_ptr<U> const& r) {
			this->indirect_value_ = 0;
			this->copy_from(r);
		}
		template <typename U>
		regular_ptr (U const& u) {
			this->copy(u);
		}
		~ regular_ptr () {
			this->clear();
		}

		template <typename U>
		regular_ptr<T>& operator = (regular_ptr<U> const& r) {
			this->copy_from(r);
		}

		void clear () {
			if (0 != this->indirect_value_)
				delete this->indirect_value_;
			this->indirect_value_ = 0;
		}

		bool valid () const { return 0 != this->indirect_value_; }

		template <typename U>
		void copy (U const& u) {
			this->clear();
			this->indirect_value_ = cloning_storage<U, T>(u);
		}
		template <typename U>
		void copy_from (regular_ptr<U> const& r) {
			this->clear();
			this->indirect_value_ = r.indirect_value_->copy();
		}

		template <typename U>
		friend bool operator == (regular_ptr<T> const& left, regular_ptr<U> const& right) {
			return left.indirect_value_->equals(right.indirect_value_);
		}
		template <typename U>
		friend bool operator != (regular_ptr<T> const& left, regular_ptr<U> const& right) {
			return left.indirect_value->not_equals(right.indirect_value_);
		}

		friend void swap (regular_ptr<T>& left, regular_ptr<T>& right) {
			detail::storage_regular<T> *tmp = left.indirect_value_;
			left.indirect_value_ = right.indirect_value_;
			right.indirect_value_ = tmp;
		}

	private:
		detail::regular_storage<T> *indirect_value_;
	};

}

#endif//VALUED_INTERFACE_POINTER
