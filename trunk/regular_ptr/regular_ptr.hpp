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
		struct interface {
			virtual ~ interface () {}
			virtual T* get_ptr () = 0;
			virtual const T* get_ptr() const = 0;
			virtual interface<T>* copy () const = 0;
			virtual bool equals (interface<T> const*) const = 0;
			virtual bool not_equals (interface<T> const*) const = 0;
		};

		template <typename T, typename U>
		struct copier : public interface<T> {
			copier () {}
			copier (U const& u) : value(u) {}
			virtual ~ copier () {}

			virtual T* get_ptr () { return &this->value; }
			virtual const T* get_ptr () const { return &this->value; }

			virtual interface<T>* copy () const {
				return new copier<T, U>(this->value);
			}

			virtual bool equals (interface<T> const* other) const {
				const U* dcother = dynamic_cast<const U*>(other->get_ptr());
				if (0 == dcother) return false;
				return *dcother == this->value;
			}
			virtual bool not_equals (interface<T> const* other) const {
				const U* dcother = dynamic_cast<const U*>(other->get_ptr());
				if (0 == dcother) return true;
				return *dcother != this->value;
			}

			U value;
		};

	}

	template <typename T>
	class regular_ptr {
	public:
		regular_ptr () : store(0) {}
		regular_ptr (regular_ptr<T> const& r) {
			this->store = 0;
			this->copy_from(r);
		}
		template <typename U>
		regular_ptr (U const& u) {
			this->store = 0;
			this->copy(u);
		}
		~ regular_ptr () {
			this->clear();
		}

		regular_ptr<T>& operator = (regular_ptr<T> const& r) {
			this->copy_from(r);
			return *this;
		}

		template <typename U>
		regular_ptr<T>& operator = (regular_ptr<U> const& r) {
			this->copy_from(r);
			return *this;
		}

		template <typename U>
		regular_ptr<T>& operator = (U const& u) {
			this->copy(u);
			return *this;
		}

		T* operator -> () { return this->store->get_ptr(); }
		const T* operator -> () const { return this->store->get_ptr(); }
		T& operator * () { return *this->store->get_ptr(); }
		const T& operator * () const { return *this->store->get_ptr(); }

		template <typename U>
		friend bool operator == (regular_ptr<T> const& left, regular_ptr<U> const& right) {
			return left.store->equals(right.store);
		}
		template <typename U>
		friend bool operator != (regular_ptr<T> const& left, regular_ptr<U> const& right) {
			return left.store->not_equals(right.store);
		}

		template <typename S>
		void print_value (S const& t) {
			std::cout << t->foo() << std::endl;
		}

		void clear () {
			if (0 != this->store) {
				delete this->store;
			}
			this->store = 0;
		}

		template <typename U>
		void copy_from (regular_ptr<U> const& r) {
			this->clear();
			this->store = r.store->copy();
		}
		template <typename U>
		void copy (U const& u) {
			this->clear();
			this->store = new detail::copier<T, U>(u);
		}

		friend void swap (regular_ptr<T>& left, regular_ptr<T>& right) {
			detail::interface<T> *tmp = left.store;
			left.store = right.store;
			right.store = tmp;
		}

	private:
		detail::interface<T> *store;
	};

}

#endif//VALUED_INTERFACE_POINTER
