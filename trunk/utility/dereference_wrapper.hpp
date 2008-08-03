#ifndef DEREFERENCE_WRAPPER
#define DEREFERENCE_WRAPPER

namespace utility {

	template <typename Value>
	struct dereference_wrapper {
		typedef Value value_type;

		dereference_wrapper () : value() {}
		dereference_wrapper (Value const& val) : value(val) {}
		dereference_wrapper (dereference_wrapper<Value> const& dw) : value(dw.value) {}

		dereference_wrapper<Value> operator = (dereference_wrapper<Value> const& dw) {
			this->value = dw.value;
			return this->value;
		}
		dereference_wrapper<Value> operator = (Value const& value) {
			this->value = value;
			return this->value;
		}

		Value& operator * () { return this->value; }
		Value const& operator * () const { return this->value; }
		Value* operator -> () { return &this->value; }
		Value const* operator -> () const { return &this->value; }
		
		Value value;
	};

}

#endif//DEREFERENCE_WRAPPER
