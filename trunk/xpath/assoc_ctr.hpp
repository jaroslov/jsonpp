#include "vpath.hpp"
#include <begin-end.hpp>
#include <map>
#include <boost/variant.hpp>

#ifndef VPATH_LIB_ASSOC_CTR
#define VPATH_LIB_ASSOC_CTR

namespace vpath {

/*

There are two ways to deal with associative containers:
  1. create an iterator facade and just return the "value" from the
      key/value pair
  2. something ... more complicated

*/

template <typename AssociativeContainer>
struct assoc_ctr_valued_iterator_facade {
  typedef assoc_ctr_valued_iterator_facade<
              AssociativeContainer>         self_type;

  typedef AssociativeContainer              ac_type;
  typedef typename ac_type::const_iterator  const_iterator;
  typedef const_iterator                    iterator;

  typedef typename ac_type::key_type        ac_key_type;
  typedef typename ac_type::mapped_type     ac_mapped_type;
  typedef typename ac_type::value_type      ac_value_type;

  typedef ac_mapped_type                    value_type;
  typedef value_type const&                 reference;
  typedef value_type const*                 pointer;

  assoc_ctr_valued_iterator_facade () {}
  assoc_ctr_valued_iterator_facade (iterator const& itr)
    : iterator_(itr) {}
  assoc_ctr_valued_iterator_facade (self_type const& st)
    : iterator_(st.iterator_) {}
  self_type& operator = (self_type const& st) {
    this->iterator_ = st.iterator_;
    return *this;
  }

  self_type& operator ++ () {
    ++this->iterator_;
    return *this;
  }
  self_type operator ++ (int) {
    self_type cp(*this);
    ++(*this);
    return cp;
  }
  ac_value_type const& operator * () const {
    return this->iterator_->second;
  }
  ac_value_type const* operator -> () const {
    return &this->iterator_->second;
  }

  friend bool operator == (self_type const& L, self_type const& R) {
    return L.iterator_ == R.iterator_;
  }
  friend bool operator != (self_type const& L, self_type const& R) {
    return L.iterator_ != R.iterator_;
  }

  iterator iterator_;
};


} // end vpath namespace

#endif//VPATH_LIB_ASSOC_CTR
