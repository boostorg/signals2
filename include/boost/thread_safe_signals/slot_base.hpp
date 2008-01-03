// Boost.Signals library

// Copyright Frank Mori Hess 2007.
// Copyright Timmo Stange 2007.
// Copyright Douglas Gregor 2001-2004. Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_TSS_SIGNALS_SLOT_BASE_HEADER
#define BOOST_TSS_SIGNALS_SLOT_BASE_HEADER

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread_safe_signals/signal_base.hpp>
#include <vector>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost
{
  class expired_slot: public bad_weak_ptr
  {
  public:
    virtual char const * what() const throw()
    {
      return "boost::expired_slot";
    }
  };

  namespace signalslib
  {
    class slot_base
    {
    public:
      typedef std::vector<boost::weak_ptr<void> > tracked_container_type;
      typedef std::vector<boost::shared_ptr<void> > locked_container_type;

      const tracked_container_type& tracked_objects() const {return _trackedObjects;}
      locked_container_type lock() const
      {
        locked_container_type locked_objects;
        tracked_container_type::const_iterator it;
        for(it = tracked_objects().begin(); it != tracked_objects().end(); ++it)
        {
          try
          {
            locked_objects.push_back(shared_ptr<void>(*it));
          }
          catch(const bad_weak_ptr &)
          {
            throw expired_slot();
          }
        }
        return locked_objects;
      }
      bool expired() const
      {
        tracked_container_type::const_iterator it;
        for(it = tracked_objects().begin(); it != tracked_objects().end(); ++it)
        {
          if(it->expired()) return true;
        }
        return false;
      }
    protected:
      void track_signal(const signalslib::signal_base &signal)
      {
        _trackedObjects.push_back(signal.lock_pimpl());
      }

      tracked_container_type _trackedObjects;
    };
  }
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SIGNALS_SLOT_BASE_HEADER
