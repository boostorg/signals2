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
    namespace detail {
      class tracked_base
      {
      public:
        virtual ~tracked_base() {}
        virtual shared_ptr<void> get_shared_ptr() const = 0;
      };
    } // end namespace detail

    // The actual wrapper for tracked shared_ptr-referenced objects.
    // (This probably needs a full smart pointer interface)
    template<class T>
    class tracked : public detail::tracked_base, public weak_ptr<T>
    {
    public:
      tracked(const weak_ptr<T>& ptr): weak_ptr<T>(ptr)
      {}
      virtual shared_ptr<void> get_shared_ptr() const {
        return lock();
      }
    };
    // Convenience functions for binders.
    template<class T>
    tracked<T> track(const boost::shared_ptr<T>& ptr) {
      return tracked<T>(ptr);
    }
    template<class T>
    tracked<T> track(const boost::reference_wrapper<shared_ptr<T> > &ref) {
      return tracked<T>(ref.get());
    }
    template<class T>
    tracked<T> track(const boost::weak_ptr<T>& ptr) {
      return tracked<T>(ptr);
    }
    template<class T>
    tracked<T> track(const boost::reference_wrapper<weak_ptr<T> > &ref) {
      return tracked<T>(ref.get());
    }
    // get_pointer lets mem_fn bind a tracked
    template<typename T>
    T* get_pointer(const signalslib::tracked<T> &tracked) {return tracked.lock().get();}
  } // end namespace BOOST_SIGNALS_NAMESPACE
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SIGNALS_TRACK_HEADER
