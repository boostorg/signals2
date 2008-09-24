/*
  boost::signals2::connection provides a handle to a signal/slot connection.

  Author: Frank Mori Hess <fmhess@users.sourceforge.net>
  Begin: 2007-01-23
*/
// Copyright Frank Mori Hess 2007-2008.
// Distributed under the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org/libs/signals2 for library home page.

#ifndef BOOST_TSS_CONNECTION_HEADER
#define BOOST_TSS_CONNECTION_HEADER

#include <boost/function.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2/slot.hpp>
#include <boost/signals2/detail/unique_lock.hpp>
#include <boost/type_traits.hpp>
#include <boost/visit_each.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

namespace boost
{
  namespace signals2
  {
    extern inline void null_deleter(const void*) {}
    namespace detail
    {
      class connection_body_base
      {
      public:
        connection_body_base():
          _connected(true)
        {
        }
        virtual ~connection_body_base() {}
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

      template<typename GroupKey, typename SlotType, typename Mutex>
      class connection_body: public connection_body_base
      {
      public:
        typedef Mutex mutex_type;
        connection_body(const SlotType &slot_in):
          slot(slot_in)
        {
        }
        virtual ~connection_body() {}
        virtual void disconnect()
        {
          unique_lock<mutex_type> lock(mutex);
          nolock_disconnect();
        }
        virtual bool connected() const
        {
          unique_lock<mutex_type> lock(mutex);
          nolock_grab_tracked_objects();
          return nolock_nograb_connected();
        }
        virtual shared_ptr<void> get_blocker()
        {
          unique_lock<mutex_type> lock(mutex);
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
      connection(const connection &other): _weak_connection_body(other._weak_connection_body)
      {}
      connection(boost::weak_ptr<detail::connection_body_base> connectionBody):
        _weak_connection_body(connectionBody)
      {}
      ~connection() {}
      void disconnect() const
      {
        boost::shared_ptr<detail::connection_body_base> connectionBody(_weak_connection_body.lock());
        if(connectionBody == 0) return;
        connectionBody->disconnect();
      }
      bool connected() const
      {
        boost::shared_ptr<detail::connection_body_base> connectionBody(_weak_connection_body.lock());
        if(connectionBody == 0) return false;
        return connectionBody->connected();
      }
      bool blocked() const
      {
        boost::shared_ptr<detail::connection_body_base> connectionBody(_weak_connection_body.lock());
        if(connectionBody == 0) return true;
        return connectionBody->blocked();
      }
      bool operator==(const connection& other) const
      {
        boost::shared_ptr<detail::connection_body_base> connectionBody(_weak_connection_body.lock());
        boost::shared_ptr<detail::connection_body_base> otherConnectionBody(other._weak_connection_body.lock());
        return connectionBody == otherConnectionBody;
      }
      bool operator<(const connection& other) const
      {
        boost::shared_ptr<detail::connection_body_base> connectionBody(_weak_connection_body.lock());
        boost::shared_ptr<detail::connection_body_base> otherConnectionBody(other._weak_connection_body.lock());
        return connectionBody < otherConnectionBody;
      }
      void swap(connection &other)
      {
        using std::swap;
        swap(_weak_connection_body, other._weak_connection_body);
      }
    private:

      boost::weak_ptr<detail::connection_body_base> _weak_connection_body;
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
        disconnect();
        connection::operator=(rhs);
        return *this;
      }
      void swap(scoped_connection &other)
      {
        connection::swap(other);
      }
    };
  }
}

#endif  // BOOST_TSS_CONNECTION_HEADER
