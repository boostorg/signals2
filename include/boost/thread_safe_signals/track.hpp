// Boost.Signals library

// Copyright Timmo Stange 2007.
// Copyright Frank Mori Hess 2007.
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_SIGNALS_TRACK_HEADER
#define BOOST_SIGNALS_TRACK_HEADER

#include <boost/ref.hpp>
#include <boost/signals/detail/config.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/noncopyable.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
  namespace signalslib {
    // The actual wrapper for tracked shared_ptr-referenced objects.
    template<class T>
    class tracked : public weak_ptr<T>
    {
    public:
      tracked(const weak_ptr<T>& ptr): weak_ptr<T>(ptr)
      {}
      operator T* ()
      {
        return shared_ptr<T>(*this).get();
      }
      operator const T* () const
      {
        return shared_ptr<const T>(*this).get();
      }
      operator shared_ptr<T> ()
      {
        return shared_ptr<T>(*this);
      }
      operator shared_ptr<const T> () const
      {
        return shared_ptr<const T>(*this);
      }
      operator T& ()
      {
        return *shared_ptr<T>(*this).get();
      }
      operator const T& ()
      {
        return *shared_ptr<const T>(*this).get();
      }
      operator shared_ptr<void> () const
      {
        return shared_ptr<void>(*this);
      }
    };
    // Convenience functions for binders.
    template<class T>
    tracked<T> track(const boost::shared_ptr<T>& ptr) {
      return tracked<T>(ptr);
    }
    template<class T>
    tracked<T> track(const boost::weak_ptr<T>& ptr) {
      return tracked<T>(ptr);
    }
    // get_pointer lets mem_fn bind a tracked
    template<typename T>
    T* get_pointer(const signalslib::tracked<T> &tracked) {return shared_ptr<T>(tracked).get();}
  } // end namespace BOOST_SIGNALS_NAMESPACE
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SIGNALS_TRACK_HEADER
