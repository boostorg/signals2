// Boost.Signals2 library

// Copyright Frank Mori Hess 2007-2008.
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_SHARED_CONNECTION_BLOCK_HPP
#define BOOST_SHARED_CONNECTION_BLOCK_HPP

#include <boost/shared_ptr.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/weak_ptr.hpp>

namespace boost
{
  namespace signals2
  {
    class shared_connection_block
    {
    public:
      shared_connection_block(const connection &conn):
        _weak_connection_body(conn._weak_connection_body)
      {
        block();
      }
      void block()
      {
        if(_blocker) return;
        boost::shared_ptr<detail::connection_body_base> connectionBody(_weak_connection_body.lock());
        if(connectionBody == 0) return;
        _blocker = connectionBody->get_blocker();
      }
      void unblock()
      {
        _blocker.reset();
      }
      bool blocking() const
      {
        return _blocker != 0;
      }
    private:
      boost::weak_ptr<detail::connection_body_base> _weak_connection_body;
      shared_ptr<void> _blocker;
    };
  }
} // end namespace boost

#endif // BOOST_SHARED_CONNECTION_BLOCK_HPP
