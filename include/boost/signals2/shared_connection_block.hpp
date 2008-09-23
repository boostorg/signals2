// Boost.Signals2 library

// Copyright Frank Mori Hess 2007-2008.
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_SHARED_CONNECTION_BLOCK_HEADER
#define BOOST_SHARED_CONNECTION_BLOCK_HEADER

#include <boost/shared_ptr.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/weak_ptr.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost
{
  namespace signals2
  {
    class shared_connection_block
    {
    public:
      shared_connection_block(connection &conn):
        _weakConnectionBody(conn._weakConnectionBody)
      {
        block();
      }
      void block()
      {
        if(_blocker) return;
        boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
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
      boost::weak_ptr<detail::ConnectionBodyBase> _weakConnectionBody;
      shared_ptr<void> _blocker;
    };
  }
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SHARED_CONNECTION_BLOCK_HEADER
