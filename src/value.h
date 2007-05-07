#ifndef _VALUE_H
#define _VALUE_H

#include "amount.h"
#include "balance.h"

namespace ledger {

namespace xml {
  class node_t;
}

// The following type is a polymorphous value type used solely for
// performance reasons.  The alternative is to compute value
// expressions (valexpr.cc) in terms of the largest data type,
// balance_t. This was found to be prohibitively expensive, especially
// when large logic chains were involved, since many temporary
// allocations would occur for every operator.  With value_t, and the
// fact that logic chains only need boolean values to continue, no
// memory allocations need to take place at all.

class value_t
  : public ordered_field_operators<value_t,
           ordered_field_operators<value_t, balance_pair_t,
           ordered_field_operators<value_t, balance_t,
           ordered_field_operators<value_t, amount_t,
           ordered_field_operators<value_t, double,
           ordered_field_operators<value_t, unsigned long,
           ordered_field_operators<value_t, long> > > > > > >
{
  char data[sizeof(balance_pair_t)];

 public:
  typedef std::vector<value_t> sequence_t;

  enum type_t {
    BOOLEAN,
    INTEGER,
    DATETIME,
    AMOUNT,
    BALANCE,
    BALANCE_PAIR,
    STRING,
    XML_NODE,
    POINTER,
    SEQUENCE
  } type;

  value_t() {
    TRACE_CTOR(value_t, "");
    *((long *) data) = 0;
    type = INTEGER;
  }

  value_t(const value_t& val) : type(INTEGER) {
    TRACE_CTOR(value_t, "copy");
    *this = val;
  }
  value_t(const bool val) {
    TRACE_CTOR(value_t, "const bool");
    *((bool *) data) = val;
    type = BOOLEAN;
  }
  value_t(const long val) {
    TRACE_CTOR(value_t, "const long");
    *((long *) data) = val;
    type = INTEGER;
  }
  value_t(const moment_t val) {
    TRACE_CTOR(value_t, "const moment_t");
    *((moment_t *) data) = val;
    type = DATETIME;
  }
  value_t(const unsigned long val) {
    TRACE_CTOR(value_t, "const unsigned long");
    new((amount_t *) data) amount_t(val);
    type = AMOUNT;
  }
  value_t(const double val) {
    TRACE_CTOR(value_t, "const double");
    new((amount_t *) data) amount_t(val);
    type = AMOUNT;
  }
  value_t(const string& val, bool literal = false) {
    TRACE_CTOR(value_t, "const string&, bool");
    if (literal) {
      type = INTEGER;
      set_string(val);
    } else {
      new((amount_t *) data) amount_t(val);
      type = AMOUNT;
    }
  }
  value_t(const char * val) {
    TRACE_CTOR(value_t, "const char *");
    new((amount_t *) data) amount_t(val);
    type = AMOUNT;
  }
  value_t(const amount_t& val) {
    TRACE_CTOR(value_t, "const amount_t&");
    new((amount_t *)data) amount_t(val);
    type = AMOUNT;
  }
  value_t(const balance_t& val) : type(INTEGER) {
    TRACE_CTOR(value_t, "const balance_t&");
    *this = val;
  }
  value_t(const balance_pair_t& val) : type(INTEGER) {
    TRACE_CTOR(value_t, "const balance_pair_t&");
    *this = val;
  }
  value_t(xml::node_t * xml_node) : type(INTEGER) { // gets set in =
    TRACE_CTOR(value_t, "xml::node_t *");
    *this = xml_node;
  }
  value_t(void * item) : type(INTEGER) { // gets set in =
    TRACE_CTOR(value_t, "void *");
    *this = item;
  }
  value_t(sequence_t * seq) : type(INTEGER) { // gets set in =
    TRACE_CTOR(value_t, "sequence_t *");
    *this = seq;
  }

  ~value_t() {
    TRACE_DTOR(value_t);
    destroy();
  }

  void destroy();
  void simplify();

  value_t& operator=(const value_t& val);
#if 0
  value_t& operator=(const bool val) {
    if ((bool *) data != &val) {
      destroy();
      *((bool *) data) = val;
      type = BOOLEAN;
    }
    return *this;
  }
  value_t& operator=(const long val) {
    if ((long *) data != &val) {
      destroy();
      *((long *) data) = val;
      type = INTEGER;
    }
    return *this;
  }
  value_t& operator=(const moment_t val) {
    if ((moment_t *) data != &val) {
      destroy();
      *((moment_t *) data) = val;
      type = DATETIME;
    }
    return *this;
  }
  value_t& operator=(const unsigned long val) {
    return *this = amount_t(val);
  }
  value_t& operator=(const double val) {
    return *this = amount_t(val);
  }
  value_t& operator=(const string& val) {
    return *this = amount_t(val);
  }
  value_t& operator=(const char * val) {
    return *this = amount_t(val);
  }
  value_t& operator=(const amount_t& val) {
    if (type == AMOUNT &&
	(amount_t *) data == &val)
      return *this;

    if (val.realzero()) {
      return *this = 0L;
    } else {
      destroy();
      new((amount_t *)data) amount_t(val);
      type = AMOUNT;
    }
    return *this;
  }
  value_t& operator=(const balance_t& val) {
    if (type == BALANCE &&
	(balance_t *) data == &val)
      return *this;

    if (val.realzero()) {
      return *this = 0L;
    }
    else if (val.amounts.size() == 1) {
      return *this = (*val.amounts.begin()).second;
    }
    else {
      destroy();
      new((balance_t *)data) balance_t(val);
      type = BALANCE;
      return *this;
    }
  }
  value_t& operator=(const balance_pair_t& val) {
    if (type == BALANCE_PAIR &&
	(balance_pair_t *) data == &val)
      return *this;

    if (val.realzero()) {
      return *this = 0L;
    }
    else if (! val.cost) {
      return *this = val.quantity;
    }
    else {
      destroy();
      new((balance_pair_t *)data) balance_pair_t(val);
      type = BALANCE_PAIR;
      return *this;
    }
  }
  value_t& operator=(xml::node_t * xml_node) {
    assert(xml_node);
    if (type == XML_NODE && *(xml::node_t **) data == xml_node)
      return *this;

    if (! xml_node) {
      type = XML_NODE;
      return *this = 0L;
    }
    else {
      destroy();
      *(xml::node_t **)data = xml_node;
      type = XML_NODE;
      return *this;
    }
  }
  value_t& operator=(void * item) {
    assert(item);
    if (type == POINTER && *(void **) data == item)
      return *this;

    if (! item) {
      type = POINTER;
      return *this = 0L;
    }
    else {
      destroy();
      *(void **)data = item;
      type = POINTER;
      return *this;
    }
  }
  value_t& operator=(sequence_t * seq) {
    assert(seq);
    if (type == SEQUENCE && *(sequence_t **) data == seq)
      return *this;

    if (! seq) {
      type = SEQUENCE;
      return *this = 0L;
    }
    else {
      destroy();
      *(sequence_t **)data = seq;
      type = SEQUENCE;
      return *this;
    }
  }
#endif

  value_t& set_string(const string& str = "") {
    if (type != STRING) {
      destroy();
      *(string **) data = new string(str);
      type = STRING;
    } else {
      **(string **) data = str;
    }
    return *this;
  }

  bool&		  to_boolean();
  long&		  to_long();
  moment_t&       to_datetime();
  amount_t&	  to_amount();
  balance_t&	  to_balance();
  balance_pair_t& to_balance_pair();
  string&	  to_string();
  xml::node_t *&  to_xml_node();
  void *&	  to_pointer();
  sequence_t *&	  to_sequence();

  value_t& operator[](const int index) {
    sequence_t * seq = to_sequence();
    assert(seq);
    return (*seq)[index];
  }

  void push_back(const value_t& val) {
    sequence_t * seq = to_sequence();
    assert(seq);
    return seq->push_back(val);
  }

  std::size_t size() const {
    sequence_t * seq = const_cast<value_t&>(*this).to_sequence();
    assert(seq);
    return seq->size();
  }

  value_t& operator+=(const value_t& val);
  value_t& operator-=(const value_t& val);
  value_t& operator*=(const value_t& val);
  value_t& operator/=(const value_t& val);

  int compare(const value_t& val) const;

  bool operator==(const value_t& val) const {
    return compare(val) == 0;
  }
#if 0
  template <typename T>
  bool operator==(const T& val) const {
    return *this == value_t(val);
  }
#endif

  bool operator<(const value_t& val) const {
    return compare(val) < 0;
  }
#if 0
  template <typename T>
  bool operator<(const T& val) const {
    return *this < value_t(val);
  }
#endif

  operator bool() const;

#if 0
  operator long() const;
  operator unsigned long() const;
  operator double() const;
  operator moment_t() const;
  operator string() const;
  operator char *() const;
  operator amount_t() const;
  operator balance_t() const;
  operator balance_pair_t() const;
#endif

  value_t operator-() const {
    return negate();
  }
  value_t negate() const {
    value_t temp = *this;
    temp.in_place_negate();
    return temp;
  }
  void in_place_negate();

  bool    is_realzero() const;
  value_t abs() const;
  void    in_place_cast(type_t cast_type);
  value_t cost() const;
  value_t annotated_price() const;
  value_t annotated_date() const;
  value_t annotated_tag() const;

  value_t cast(type_t cast_type) const {
    value_t temp(*this);
    temp.in_place_cast(cast_type);
    return temp;
  }

  value_t strip_annotations(const bool keep_price = amount_t::keep_price,
			    const bool keep_date  = amount_t::keep_date,
			    const bool keep_tag   = amount_t::keep_tag) const;

  value_t& add(const amount_t& amount,
	       const optional<amount_t>& cost = optional<amount_t>());
  value_t  value(const optional<moment_t>& moment =
		 optional<moment_t>()) const;

  void    in_place_reduce();
  value_t reduce() const {
    value_t temp(*this);
    temp.in_place_reduce();
    return temp;
  }

  value_t round() const;
  value_t unround() const;

  void print(std::ostream& out, const int first_width,
	     const int latter_width = -1) const;

  friend std::ostream& operator<<(std::ostream& out, const value_t& val);
};

#if 0
template <typename T>
value_t::operator T() const
{
  switch (type) {
  case BOOLEAN:
    return *(bool *) data;
  case INTEGER:
    return *(long *) data;
  case DATETIME:
    return *(moment_t *) data;
  case AMOUNT:
    return *(amount_t *) data;
  case BALANCE:
    return *(balance_t *) data;
  case STRING:
    return **(string **) data;
  case XML_NODE:
    return *(xml::node_t **) data;
  case POINTER:
    return *(void **) data;
  case SEQUENCE:
    return *(sequence_t **) data;

  default:
    assert(0);
    break;
  }
  assert(0);
  return 0;
}

template <> value_t::operator bool() const;
template <> value_t::operator long() const;
template <> value_t::operator moment_t() const;
template <> value_t::operator double() const;
template <> value_t::operator string() const;
#endif

std::ostream& operator<<(std::ostream& out, const value_t& val);

#if 0
class value_context : public error_context
{
  value_t * bal;
 public:
  value_context(const value_t& _bal,
		const string& desc = "") throw();
  virtual ~value_context() throw();

  virtual void describe(std::ostream& out) const throw();
};
#endif

DECLARE_EXCEPTION(value_error);

} // namespace ledger

#endif // _VALUE_H
