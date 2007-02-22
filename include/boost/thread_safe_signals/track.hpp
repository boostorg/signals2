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

#include <boost/smart_ptr.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
  namespace signalslib {
    // The actual wrapper for tracked shared_ptr-managed objects.
    template<typename T>
    class tracked
    {
    public:
      typedef T value_type;

      tracked(const T &value, const shared_ptr<void>& tracked_ptr):
        _value(value), _tracked_ptr(tracked_ptr)
      {}
      // implicit conversions so tracked objects can be bound with bind
      operator T& ()
      {
        return _value;
      }
      operator const T& () const
      {
        return _value;
      }

      const weak_ptr<void>& get_tracked_ptr() const
      {
        return _tracked_ptr;
      }
    private:
      T _value;
      weak_ptr<void> _tracked_ptr;
    };

    template<typename T>
    class tracked_shared_ptr: public tracked<weak_ptr<T> >
    {
    public:
      tracked_shared_ptr(const weak_ptr<T>& ptr):
        tracked<weak_ptr<T> >(ptr, ptr.lock())
      {}
      operator shared_ptr<T> () const
      {
        return shared_ptr<T>(static_cast<const weak_ptr<T> &>(*this));
      }
    };

   // Convenience functions for binders.
    template<typename T>
    tracked_shared_ptr<T> track(const shared_ptr<T>& ptr) {
      return tracked_shared_ptr<T>(ptr);
    }
    template<typename T>
    tracked_shared_ptr<T> track(const weak_ptr<T>& ptr) {
      return tracked_shared_ptr<T>(ptr);
    }
    template<typename T>
    tracked<T> track(const T &value, const shared_ptr<void> &tracked_ptr) {
      return tracked<T>(value, tracked_ptr);
    }
    // get_pointer lets mem_fn bind a tracked
    template<typename T>
    T* get_pointer(const signalslib::tracked<T*> &tracked) {return tracked;}
    template<typename T>
    T* get_pointer(const signalslib::tracked<weak_ptr<T> > &tracked) {return shared_ptr<T>(static_cast<weak_ptr<T> >(tracked)).get();}
    // handles T=shared_ptr or similar case
    template<typename T>
    typename T::pointer get_pointer(const signalslib::tracked<T> &tracked) {return static_cast<const T&>(tracked).get();}
  } // end namespace BOOST_SIGNALS_NAMESPACE
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SIGNALS_TRACK_HEADER
