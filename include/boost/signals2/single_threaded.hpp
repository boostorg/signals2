// Boost.Signals2 library

// Copyright Frank Mori Hess 2007-2008.
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_SIGNALS_SINGLE_THREADED_MODEL_HEADER
#define BOOST_SIGNALS_SINGLE_THREADED_MODEL_HEADER

namespace boost {
  namespace signals2 {
    namespace detail
    {
      class null_mutex;

      class null_scoped_lock
      {
      public:
        null_scoped_lock(null_mutex &)
        {
        }
      };
      class null_mutex
      {
      public:
        typedef null_scoped_lock scoped_lock;
      };
    }

    class single_threaded
    {
    public:
      typedef detail::null_mutex mutex_type;
    };
  } // end namespace signals2
} // end namespace boost

#endif // BOOST_SIGNALS_SINGLE_THREADED_MODEL_HEADER

