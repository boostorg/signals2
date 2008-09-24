// A simple framework for creating objects with postconstructors.
// The objects must inherit from boost::signals2::postconstructible, and
// have their lifetimes managed by
// boost::shared_ptr created with the boost::signals2::deconstruct_ptr()
// function.
//
// Copyright Frank Mori Hess 2007-2008.
//
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_POSTCONSTRUCTIBLE_HEADER
#define BOOST_POSTCONSTRUCTIBLE_HEADER

namespace boost
{
  namespace signals2
  {
    class postconstructible;
    namespace detail
    {
      void do_postconstruct(const boost::signals2::postconstructible *ptr);
    }

    class postconstructible
    {
    public:
      friend void detail::do_postconstruct(const boost::signals2::postconstructible *ptr);
    protected:
      postconstructible() {}
      virtual ~postconstructible() {}
      virtual void postconstruct() {}
    };
  }
}

#endif
