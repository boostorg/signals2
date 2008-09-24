// Boost.Signals library

// Copyright (C) Douglas Gregor 2001-2006. Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/shared_ptr.hpp>
#include <boost/test/minimal.hpp>
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

int test_main(int, char*[])
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
  return 0;
}
