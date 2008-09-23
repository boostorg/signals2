// A factory function for creating a shared_ptr that enhances the plain
// shared_ptr constructors by adding support for postconstructors
// and predestructors through the boost::signals2::postconstructible and
// boost::signals2::predestructible base classes.
//
// Copyright Frank Mori Hess 2007-2008.
//
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECONSTRUCT_PTR_HEADER
#define BOOST_DECONSTRUCT_PTR_HEADER

#include <boost/checked_delete.hpp>
#include <boost/postconstructible.hpp>
#include <boost/predestructible.hpp>
#include <boost/shared_ptr.hpp>

namespace boost
{
  namespace signals2
  {
    namespace deconstruct_detail
    {
      extern inline void do_postconstruct(const boost::postconstructible *ptr)
      {
        boost::postconstructible *nonconst_ptr = const_cast<boost::postconstructible*>(ptr);
        nonconst_ptr->postconstruct();
      }
      extern inline void do_postconstruct(...)
      {
      }
    }
    template<typename T> class predestructing_deleter
    {
    public:
      void operator()(const T *ptr) const
      {
        m_predestruct(ptr);
        checked_delete(ptr);
      }
    private:
      static void m_predestruct(...)
      {
      }
      static void m_predestruct(const boost::predestructible *ptr)
      {
        boost::predestructible *nonconst_ptr = const_cast<boost::predestructible*>(ptr);
        nonconst_ptr->predestruct();
      }
    };

    template<typename T>
    shared_ptr<T> deconstruct_ptr(T *ptr)
    {
      if(ptr == 0) return shared_ptr<T>(ptr);
      shared_ptr<T> shared(ptr, boost::predestructing_deleter<T>());
      deconstruct_detail::do_postconstruct(ptr);
      return shared;
    }
    template<typename T, typename D>
    shared_ptr<T> deconstruct_ptr(T *ptr, D deleter)
    {
      shared_ptr<T> shared(ptr, deleter);
      if(ptr == 0) return shared;
      deconstruct_detail::do_postconstruct(ptr);
      return shared;
    }
  }
}

#endif
