// Boost.Signals library

// Copyright Douglas Gregor 2001-2004.
// Copyright Frank Mori Hess 2007. Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_TSS_SIGNALS_COMMON_HEADER
#define BOOST_TSS_SIGNALS_COMMON_HEADER

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
  namespace signalslib {
    namespace detail {
      // The unusable class is a placeholder for unused function arguments
      // It is also completely unusable except that it constructable from
      // anything. This helps compilers without partial specialization
      // handle slots returning void.
      struct unusable {
        unusable() {}
      };

      // Determine the result type of a slot call
      template<typename R>
      struct slot_result_type_wrapper {
        typedef R type;
      };

      template<>
      struct slot_result_type_wrapper<void> {
        typedef unusable type;
      };
    } // end namespace detail
  } // end namespace BOOST_SIGNALS_NAMESPACE
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TSS_SIGNALS_COMMON_HEADER
