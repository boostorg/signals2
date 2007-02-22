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
    /* a weak_ptr that supports various implicit conversions, so it
    binds to more types of parameters with boost::bind */
    template<typename T>
    class convertible_weak_ptr: public weak_ptr<T>
    {
    public:
      convertible_weak_ptr(const weak_ptr<T> &ptr): weak_ptr<T>(ptr)
      {}
      convertible_weak_ptr(const shared_ptr<T> &ptr): weak_ptr<T>(ptr)
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
    };
    // The actual wrapper for tracked shared_ptr-managed objects.
    template<typename T>
    class tracked
    {
    public:
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
    // Convenience functions for binders.
    template<typename T>
    tracked<convertible_weak_ptr<T> > track(const shared_ptr<T>& ptr) {
      return tracked<convertible_weak_ptr<T> >(weak_ptr<T>(ptr), ptr);
    }
    template<typename T>
    tracked<convertible_weak_ptr<T> > track(const weak_ptr<T>& ptr) {
      return tracked<convertible_weak_ptr<T> >(ptr, ptr.lock());
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
    template<typename T>
    T* get_pointer(const signalslib::tracked<convertible_weak_ptr<T> > &tracked) {return shared_ptr<T>(static_cast<weak_ptr<T> >(tracked)).get();}
  } // end namespace BOOST_SIGNALS_NAMESPACE
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SIGNALS_TRACK_HEADER
