/*
 *  Copyright (c) 2015, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 */

#include <fatal/type/variant_traits.h>

#include <fatal/type/call_traits.h>

#include <fatal/test/driver.h>

#include <memory>
#include <type_traits>
#include <utility>

namespace fatal {

class poor_mans_variant {
  union storage {
    int i;
    double d;
    bool b;
  };

public:
  enum class id { i, d, b, empty };

  int get_i() const & { return storage_.i; }
  double get_d() const & { return storage_.d; }
  bool get_b() const & { return storage_.b; }

  int &get_i() & { return storage_.i; }
  double &get_d() & { return storage_.d; }
  bool &get_b() & { return storage_.b; }

  int &&get_i() && { return std::move(storage_.i); }
  double &&get_d() && { return std::move(storage_.d); }
  bool &&get_b() && { return std::move(storage_.b); }

  void set_i(int i) { storage_.i = i; }
  void set_d(double d) { storage_.d = d; }
  void set_b(bool b) { storage_.b = b; }

  bool empty() const { return id_ == id::empty; }
  void clear() { id_ = id::empty; }

  storage const *operator ->() const { return std::addressof(storage_); }

private:
  id id_ = id::empty;
  storage storage_;
};

class poor_mans_variant_traits {
  struct get {
    FATAL_CALL_TRAITS(i, get_i);
    FATAL_CALL_TRAITS(d, get_d);
    FATAL_CALL_TRAITS(b, get_b);
  };

  struct set {
    FATAL_CALL_TRAITS(i, set_i);
    FATAL_CALL_TRAITS(d, set_d);
    FATAL_CALL_TRAITS(b, set_b);
  };

public:
  using type = poor_mans_variant;
  using id = type::id;

  struct names {
    FATAL_STR(i, "i");
    FATAL_STR(d, "d");
    FATAL_STR(b, "b");
  };

  struct ids {
    using i = std::integral_constant<id, id::i>;
    using d = std::integral_constant<id, id::d>;
    using b = std::integral_constant<id, id::b>;
  };

  using descriptors = fatal::type_list<
    variant_type_descriptor<
      int,
      ids::i,
      names::i,
      get::i::member_function,
      set::i::member_function
    >,
    variant_type_descriptor<
      double,
      ids::d,
      names::d,
      get::d::member_function,
      set::d::member_function
    >,
    variant_type_descriptor<
      bool,
      ids::b,
      names::b,
      get::b::member_function,
      set::b::member_function
    >
  >;
};

FATAL_REGISTER_VARIANT_TRAITS(poor_mans_variant_traits);

FATAL_TEST(poor_mans_variant, types) {
  using type = poor_mans_variant;
  using names = poor_mans_variant_traits::names;
  using ids = poor_mans_variant_traits::ids;
  using traits = variant_traits<type>;

  FATAL_EXPECT_SAME<type, traits::type>();
  FATAL_EXPECT_SAME<type::id, traits::id>();

  FATAL_EXPECT_SAME<names::i, traits::names::i>();
  FATAL_EXPECT_SAME<names::d, traits::names::d>();
  FATAL_EXPECT_SAME<names::b, traits::names::b>();

  FATAL_EXPECT_SAME<ids::i, traits::ids::i>();
  FATAL_EXPECT_SAME<ids::d, traits::ids::d>();
  FATAL_EXPECT_SAME<ids::b, traits::ids::b>();

  FATAL_EXPECT_SAME<
    poor_mans_variant_traits::descriptors,
    traits::descriptors
  >();
}

FATAL_TEST(poor_mans_variant, by_name) {
  using type = poor_mans_variant;
  using names = poor_mans_variant_traits::names;
  using ids = poor_mans_variant_traits::ids;
  using traits = variant_traits<type>::by_name;

  FATAL_EXPECT_SAME<type_list<names::i, names::d, names::b>, traits::tags>();

  FATAL_EXPECT_SAME<names::i, traits::name<names::i>>();
  FATAL_EXPECT_SAME<names::d, traits::name<names::d>>();
  FATAL_EXPECT_SAME<names::b, traits::name<names::b>>();

  FATAL_EXPECT_SAME<ids::i, traits::id<names::i>>();
  FATAL_EXPECT_SAME<ids::d, traits::id<names::d>>();
  FATAL_EXPECT_SAME<ids::b, traits::id<names::b>>();

  FATAL_EXPECT_SAME<int, traits::type<names::i>>();
  FATAL_EXPECT_SAME<double, traits::type<names::d>>();
  FATAL_EXPECT_SAME<bool, traits::type<names::b>>();

  type v;
  type const &c = v;
  type &&r = std::move(v);

  v.set_i(10);
  FATAL_EXPECT_EQ(10, traits::get<names::i>(v));
  FATAL_EXPECT_EQ(10, traits::get<names::i>(c));
  FATAL_EXPECT_EQ(10, traits::get<names::i>(r));

  v.set_d(5.6);
  FATAL_EXPECT_EQ(5.6, traits::get<names::d>(v));
  FATAL_EXPECT_EQ(5.6, traits::get<names::d>(c));
  FATAL_EXPECT_EQ(5.6, traits::get<names::d>(r));

  v.set_b(true);
  FATAL_EXPECT_EQ(true, traits::get<names::b>(v));
  FATAL_EXPECT_EQ(true, traits::get<names::b>(c));
  FATAL_EXPECT_EQ(true, traits::get<names::b>(r));

  traits::set<names::i>(v, 97);
  FATAL_EXPECT_EQ(97, v->i);
  FATAL_EXPECT_EQ(97, c->i);
  FATAL_EXPECT_EQ(97, r->i);

  traits::set<names::d>(v, 7.2);
  FATAL_EXPECT_EQ(7.2, v->d);
  FATAL_EXPECT_EQ(7.2, c->d);
  FATAL_EXPECT_EQ(7.2, r->d);

  traits::set<names::b>(v, false);
  FATAL_EXPECT_EQ(false, v->b);
  FATAL_EXPECT_EQ(false, c->b);
  FATAL_EXPECT_EQ(false, r->b);
}

FATAL_TEST(poor_mans_variant, by_id) {
  using type = poor_mans_variant;
  using names = poor_mans_variant_traits::names;
  using ids = poor_mans_variant_traits::ids;
  using traits = variant_traits<type>::by_id;

  FATAL_EXPECT_SAME<type_list<ids::i, ids::d, ids::b>, traits::tags>();

  FATAL_EXPECT_SAME<names::i, traits::name<ids::i>>();
  FATAL_EXPECT_SAME<names::d, traits::name<ids::d>>();
  FATAL_EXPECT_SAME<names::b, traits::name<ids::b>>();

  FATAL_EXPECT_SAME<ids::i, traits::id<ids::i>>();
  FATAL_EXPECT_SAME<ids::d, traits::id<ids::d>>();
  FATAL_EXPECT_SAME<ids::b, traits::id<ids::b>>();

  FATAL_EXPECT_SAME<int, traits::type<ids::i>>();
  FATAL_EXPECT_SAME<double, traits::type<ids::d>>();
  FATAL_EXPECT_SAME<bool, traits::type<ids::b>>();

  type v;
  type const &c = v;
  type &&r = std::move(v);

  v.set_i(10);
  FATAL_EXPECT_EQ(10, traits::get<ids::i>(v));
  FATAL_EXPECT_EQ(10, traits::get<ids::i>(c));
  FATAL_EXPECT_EQ(10, traits::get<ids::i>(r));

  v.set_d(5.6);
  FATAL_EXPECT_EQ(5.6, traits::get<ids::d>(v));
  FATAL_EXPECT_EQ(5.6, traits::get<ids::d>(c));
  FATAL_EXPECT_EQ(5.6, traits::get<ids::d>(r));

  v.set_b(true);
  FATAL_EXPECT_EQ(true, traits::get<ids::b>(v));
  FATAL_EXPECT_EQ(true, traits::get<ids::b>(c));
  FATAL_EXPECT_EQ(true, traits::get<ids::b>(r));

  traits::set<ids::i>(v, 97);
  FATAL_EXPECT_EQ(97, v->i);
  FATAL_EXPECT_EQ(97, c->i);
  FATAL_EXPECT_EQ(97, r->i);

  traits::set<ids::d>(v, 7.2);
  FATAL_EXPECT_EQ(7.2, v->d);
  FATAL_EXPECT_EQ(7.2, c->d);
  FATAL_EXPECT_EQ(7.2, r->d);

  traits::set<ids::b>(v, false);
  FATAL_EXPECT_EQ(false, v->b);
  FATAL_EXPECT_EQ(false, c->b);
  FATAL_EXPECT_EQ(false, r->b);
}

FATAL_TEST(poor_mans_variant, by_type) {
  using type = poor_mans_variant;
  using names = poor_mans_variant_traits::names;
  using ids = poor_mans_variant_traits::ids;
  using traits = variant_traits<type>::by_type;

  FATAL_EXPECT_SAME<type_list<int, double, bool>, traits::tags>();

  FATAL_EXPECT_SAME<names::i, traits::name<int>>();
  FATAL_EXPECT_SAME<names::d, traits::name<double>>();
  FATAL_EXPECT_SAME<names::b, traits::name<bool>>();

  FATAL_EXPECT_SAME<ids::i, traits::id<int>>();
  FATAL_EXPECT_SAME<ids::d, traits::id<double>>();
  FATAL_EXPECT_SAME<ids::b, traits::id<bool>>();

  FATAL_EXPECT_SAME<int, traits::type<int>>();
  FATAL_EXPECT_SAME<double, traits::type<double>>();
  FATAL_EXPECT_SAME<bool, traits::type<bool>>();

  type v;
  type const &c = v;
  type &&r = std::move(v);

  v.set_i(10);
  FATAL_EXPECT_EQ(10, traits::get<int>(v));
  FATAL_EXPECT_EQ(10, traits::get<int>(c));
  FATAL_EXPECT_EQ(10, traits::get<int>(r));

  v.set_d(5.6);
  FATAL_EXPECT_EQ(5.6, traits::get<double>(v));
  FATAL_EXPECT_EQ(5.6, traits::get<double>(c));
  FATAL_EXPECT_EQ(5.6, traits::get<double>(r));

  v.set_b(true);
  FATAL_EXPECT_EQ(true, traits::get<bool>(v));
  FATAL_EXPECT_EQ(true, traits::get<bool>(c));
  FATAL_EXPECT_EQ(true, traits::get<bool>(r));

  traits::set<int>(v, 97);
  FATAL_EXPECT_EQ(97, v->i);
  FATAL_EXPECT_EQ(97, c->i);
  FATAL_EXPECT_EQ(97, r->i);

  traits::set<double>(v, 7.2);
  FATAL_EXPECT_EQ(7.2, v->d);
  FATAL_EXPECT_EQ(7.2, c->d);
  FATAL_EXPECT_EQ(7.2, r->d);

  traits::set<bool>(v, false);
  FATAL_EXPECT_EQ(false, v->b);
  FATAL_EXPECT_EQ(false, c->b);
  FATAL_EXPECT_EQ(false, r->b);
}

  // TODO: empty
  // TODO: clear

} // namespace fatal {