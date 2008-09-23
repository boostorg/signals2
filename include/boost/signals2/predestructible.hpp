// A simple framework for creating objects with predestructors.
// The objects must inherit from boost::signals2::predestructible, and
// have their lifetimes managed by
// boost::shared_ptr created with the boost::signals2::deconstruct_ptr()
// function.
//
// Copyright Frank Mori Hess 2007-2008.
//
//Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_PREDESTRUCTIBLE_HEADER
#define BOOST_PREDESTRUCTIBLE_HEADER

namespace boost
{
  namespace signals2
  {
    class predestructible
    {
    public:
      virtual void predestruct() {}
    protected:
      predestructible() {}
      virtual ~predestructible() {}
    };
  }
}

#endif
