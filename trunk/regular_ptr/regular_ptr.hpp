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

namespace viptr {

namespace detail {

template <typename Interface>
class valued_interface_base {
public:
  virtual Interface* get_ptr () = 0;
  virtual Interface const* get_ptr () const = 0;
  virtual valued_interface_base<Interface>* clone () const = 0;
  virtual ~ valued_interface_base () {}
};

template <typename Interface, typename Value>
class valued_interface : public valued_interface_base<Interface> {
public:
  valued_interface () {}
  valued_interface (Value const& val) : value_m(val) {}
  valued_interface (valued_interface const& vi)
    : value_m(vi.value_m) {}
  virtual ~ valued_interface () {}
  void operator = (valued_interface const& vi) {
    this->value_m = vi.value_m;
  }
  void operator = (Value const& val) {
    this->value_m = val;
  }
  virtual Interface* get_ptr () { return &this->value_m; }
  virtual Interface const* get_ptr () const { return &this->value_m; }
  virtual valued_interface_base<Interface>* clone () const {
    return new valued_interface<Interface, Value>(this->value_m);
  }
private:
  Value value_m;
};

}

template <typename Interface>
class valued_interface_ptr {
public:
  valued_interface_ptr () : ivalue_m(0) {}
  template <typename Value>
  valued_interface_ptr (Value const& v)
    : ivalue_m(new detail::valued_interface<Interface, Value>(v)) {}
  valued_interface_ptr (valued_interface_ptr const& viptr)
    : ivalue_m(viptr.clone()) {}
  virtual ~ valued_interface_ptr () {
    this->clear();
  }
  void operator = (valued_interface_ptr const& viptr) {
    this->copy(viptr);
  }
  Interface* operator -> () { return this->ivalue_m->get_ptr(); }
  Interface const* operator -> () const { return this->ivalue_m->get_ptr(); }
  Interface& operator * () { return *this->operator->(); }
  Interface const& operator * () const { return *this->operator->(); }
  void clear () {
    if (0 != this->ivalue_m)
      delete this->ivalue_m;
    this->ivalue_m = 0;
  }
  bool empty () const {
    return 0 == this->ivalue_m;
  }
  template <typename Value>
  void set (Value const& v) {
    this->clear();
    this->ivalue_m = new detail::valued_interface<Interface, Value>(v);
  }
protected:
  void copy (valued_interface_ptr const& viptr) {
    this->clear();
    this->ivalue_m = viptr.clone();
  }
  detail::valued_interface_base<Interface>* clone () const {
    return this->ivalue_m->clone();
  }
private:
  detail::valued_interface_base<Interface> *ivalue_m;
};

}

#endif//VALUED_INTERFACE_POINTER
