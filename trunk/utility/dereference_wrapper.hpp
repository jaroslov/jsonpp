#ifndef DEREFERENCE_WRAPPER
#define DEREFERENCE_WRAPPER

namespace utility {

	template <typename Value>
	struct dereference_wrapper {
		dereference_wrapper () : value() {}
		dereference_wrapper (Value const& val) : value(val) {}
		dereference_wrapper (dereference_wrapper<Value> const& dw) : value(dw.value) {}

		Value value;
	};

}

#endif//DEREFERENCE_WRAPPER
