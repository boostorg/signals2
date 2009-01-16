// Tests for boost::signals2::deconstruct_ptr and friends

// Copyright Frank Mori Hess 2007-2008.
// Distributed under the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org/libs/signals2 for library home page.

#include <boost/shared_ptr.hpp>
#include <boost/test/minimal.hpp>
#include <boost/signals2/deconstruct.hpp>
#include <boost/signals2/deconstruct_ptr.hpp>

class X: public boost::signals2::postconstructible {
public:
  X(): _postconstructed(false)
  {}
  ~X()
  {
    BOOST_CHECK(_postconstructed);
  }
protected:
  virtual void postconstruct()
  {
    _postconstructed = true;
  }
  bool _postconstructed;
};

class Y: public boost::signals2::predestructible {
public:
  Y(): _predestructed(false)
  {}
  ~Y()
  {
    BOOST_CHECK(_predestructed);
  }
protected:
  virtual void predestruct()
  {
    _predestructed = true;
  }
  bool _predestructed;
};

class Z: public X, public Y
{};

class by_deconstruct_only: public boost::signals2::postconstructible {
public:
  ~by_deconstruct_only()
  {
    BOOST_CHECK(_postconstructed);
  }
protected:
  virtual void postconstruct()
  {
    _postconstructed = true;
  }
  bool _postconstructed;
private:
  friend class boost::signals2::deconstruct_access;
  by_deconstruct_only(int): _postconstructed(false)
  {}
};

void deconstruct_ptr_test()
{
  {
    boost::shared_ptr<X> x = boost::signals2::deconstruct_ptr(new X);
  }
  {
    boost::shared_ptr<Y> x = boost::signals2::deconstruct_ptr(new Y);
  }
  {
    boost::shared_ptr<Z> z = boost::signals2::deconstruct_ptr(new Z);
  }
}

void deconstruct_test()
{
  {
    boost::shared_ptr<X> x = boost::signals2::deconstruct<X>();
  }
  {
    boost::shared_ptr<Y> x = boost::signals2::deconstruct<Y>();
  }
  {
    boost::shared_ptr<Z> z = boost::signals2::deconstruct<Z>();
  }
  {
    boost::shared_ptr<by_deconstruct_only> a = boost::signals2::deconstruct<by_deconstruct_only>(1);
  }
}

int test_main(int, char*[])
{
  deconstruct_ptr_test();
  deconstruct_test();
  return 0;
}
