// Boost.Signals library

// Copyright Douglas Gregor 2001-2004.
// Copyright Frank Mori Hess 2007. Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_TSS_SIGNALS_COMMON_HEADER
#define BOOST_TSS_SIGNALS_COMMON_HEADER

#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/ref.hpp>
#include <boost/signals2/signal_base.hpp>
#include <boost/type_traits.hpp>

namespace boost {
  namespace signals2 {
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

      // Determine if the given type T is a signal
      template<typename T>
      class is_signal: public mpl::bool_<is_base_of<signal_base, T>::value>
      {};

      // A slot can be a signal, a reference to a function object, or a
      // function object.
      struct signal_tag {};
      struct reference_tag {};
      struct value_tag {};

      // Classify the given slot as a signal, a reference-to-slot, or a
      // standard slot
      template<typename S>
      class get_slot_tag {
        typedef typename mpl::if_<is_signal<S>,
          signal_tag, value_tag>::type signal_or_value;
      public:
        typedef typename mpl::if_<is_reference_wrapper<S>,
                            reference_tag,
                            signal_or_value>::type type;
      };
    } // end namespace detail
  } // end namespace signals2
} // end namespace boost

#endif // BOOST_TSS_SIGNALS_COMMON_HEADER
