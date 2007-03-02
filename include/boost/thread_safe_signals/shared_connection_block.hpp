// Boost.Signals library

// Copyright Frank Mori Hess 2007.
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_SHARED_CONNECTION_BLOCK_HEADER
#define BOOST_SHARED_CONNECTION_BLOCK_HEADER

#include <boost/shared_ptr.hpp>
#include <boost/thread_safe_signals/connection.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost
{
	namespace signalslib
	{
		class shared_connection_block
		{
		public:
			shared_connection_block(connection &conn):
				_blocker(conn.get_blocker())
			{}
			void unblock()
			{
				_blocker.reset();
			}
		private:
			shared_ptr<void> _blocker;
		};
	}
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SHARED_CONNECTION_BLOCK_HEADER
