// Boost.Signals library

// Copyright Frank Mori Hess 2007-2008.
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_SIGNALS_AUTO_THREADED_MODEL_HEADER
#define BOOST_SIGNALS_AUTO_THREADED_MODEL_HEADER

#include <boost/detail/lightweight_mutex.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
  namespace signals2 {
    class auto_threaded
    {
    public:
      typedef boost::detail::lightweight_mutex mutex_type;
    };
  } // end namespace signals2
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SIGNALS_AUTO_THREADED_MODEL_HEADER

