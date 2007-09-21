/*

  Author: Frank Hess <frank.hess@nist.gov>
  Begin: 2007-01-23
*/
/* This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to title 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and is
 * in the public domain. This is an experimental system. NIST assumes no
 * responsibility whatsoever for its use by other parties, and makes no
 * guarantees, expressed or implied, about its quality, reliability, or
 * any other characteristic. We would appreciate acknowledgement if the
 * software is used.
 */
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TSS_CONNECTION_HEADER
#define BOOST_TSS_CONNECTION_HEADER

#include <boost/function.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread_safe_signals/slot.hpp>
#include <boost/type_traits.hpp>
#include <boost/visit_each.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost
{
  namespace signalslib
  {
    extern inline void null_deleter(const void*) {}
    namespace detail
    {
      class ConnectionBodyBase
      {
      public:
        ConnectionBodyBase():
          _connected(true)
        {
        }
        virtual ~ConnectionBodyBase() {}
        virtual void disconnect() = 0;
        void nolock_disconnect()
        {
          _connected = false;
        }
        virtual bool connected() const = 0;
        virtual shared_ptr<void> get_blocker() = 0;
        bool blocked() const
        {
          return !_weak_blocker.expired();
        }
        bool nolock_nograb_blocked() const
        {
          return nolock_nograb_connected() == false || blocked();
        }
        bool nolock_nograb_connected() const {return _connected;}
      protected:

        mutable bool _connected;
        weak_ptr<void> _weak_blocker;
      };

      template<typename GroupKey, typename SlotType, typename ThreadingModel>
      class ConnectionBody: public ConnectionBodyBase
      {
      public:
        typedef typename ThreadingModel::mutex_type mutex_type;
        ConnectionBody(const SlotType &slot_in):
          slot(slot_in)
        {
        }
        virtual ~ConnectionBody() {}
        virtual void disconnect()
        {
          typename mutex_type::scoped_lock lock(mutex);
          nolock_disconnect();
        }
        virtual bool connected() const
        {
          typename mutex_type::scoped_lock lock(mutex);
          nolock_grab_tracked_objects();
          return nolock_nograb_connected();
        }
        virtual shared_ptr<void> get_blocker()
        {
          typename mutex_type::scoped_lock lock(mutex);
          shared_ptr<void> blocker = _weak_blocker.lock();
          if(blocker == 0)
          {
            blocker.reset(this, &null_deleter);
            _weak_blocker = blocker;
          }
          return blocker;
        }
        const GroupKey& group_key() const {return _group_key;}
        void set_group_key(const GroupKey &key) {_group_key = key;}
        bool nolock_slot_expired() const
        {
          bool expired = slot.expired();
          if(expired == true)
          {
            _connected = false;
          }
          return expired;
        }
        typename slot_base::locked_container_type nolock_grab_tracked_objects() const
        {
          slot_base::locked_container_type locked_objects;
          try
          {
            locked_objects = slot.lock();
          }
          catch(const expired_slot &)
          {
            _connected = false;
            return locked_objects;
          }
          return locked_objects;
        }
        SlotType slot;
        mutable mutex_type mutex;
      private:
        GroupKey _group_key;
      };
    }

    class shared_connection_block;

    class connection
    {
    public:
      friend class shared_connection_block;

      connection() {}
      connection(const connection &other): _weakConnectionBody(other._weakConnectionBody)
      {}
      connection(boost::weak_ptr<detail::ConnectionBodyBase> connectionBody):
        _weakConnectionBody(connectionBody)
      {}
      ~connection() {}
      void disconnect() const
      {
        boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
        if(connectionBody == 0) return;
        connectionBody->disconnect();
      }
      bool connected() const
      {
        boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
        if(connectionBody == 0) return false;
        return connectionBody->connected();
      }
      bool blocked() const
      {
        boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
        if(connectionBody == 0) return true;
        return connectionBody->blocked();
      }
      bool operator==(const connection& other) const
      {
        boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
        boost::shared_ptr<detail::ConnectionBodyBase> otherConnectionBody(other._weakConnectionBody.lock());
        return connectionBody == otherConnectionBody;
      }
      bool operator<(const connection& other) const
      {
        boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
        boost::shared_ptr<detail::ConnectionBodyBase> otherConnectionBody(other._weakConnectionBody.lock());
        return connectionBody < otherConnectionBody;
      }
      void swap(connection &other)
      {
        std::swap(_weakConnectionBody, other._weakConnectionBody);
      }
    private:

      boost::weak_ptr<detail::ConnectionBodyBase> _weakConnectionBody;
    };

    class scoped_connection: public connection
    {
    public:
      scoped_connection() {}
      scoped_connection(const connection &other): connection(other)
      {}
      ~scoped_connection()
      {
        disconnect();
      }
      const scoped_connection& operator=(const connection &rhs)
      {
        boost::signalslib::connection::operator=(rhs);
        return *this;
      }
      void swap(scoped_connection &other)
      {
        connection::swap(other);
      }
    };
  }
}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif  // BOOST_TSS_CONNECTION_HEADER
