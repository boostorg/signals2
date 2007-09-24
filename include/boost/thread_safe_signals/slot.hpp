// Boost.Signals library

// Copyright Frank Mori Hess 2007.
// Copyright Timmo Stange 2007.
// Copyright Douglas Gregor 2001-2004. Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_TSS_SIGNALS_SLOT_HEADER
#define BOOST_TSS_SIGNALS_SLOT_HEADER

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/ref.hpp>
#include <boost/thread_safe_signals/detail/signals_common.hpp>
#include <boost/thread_safe_signals/detail/signals_common_macros.hpp>
#include <boost/thread_safe_signals/slot_base.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost
{
  namespace signalslib
  {
    namespace detail
    {
      // Get the slot so that it can be copied
      template<typename F>
      typename F::weak_signal_type
      get_invocable_slot(const F &signal, signalslib::detail::signal_tag)
      { return typename F::weak_signal_type(signal); }

      template<typename F>
      const F&
      get_invocable_slot(const F& f, signalslib::detail::reference_tag)
      { return f; }

      template<typename F>
      const F&
      get_invocable_slot(const F& f, signalslib::detail::value_tag)
      { return f; }

      // Determines the type of the slot - is it a signal, a reference to a
      // slot or just a normal slot.
      template<typename F>
      typename signalslib::detail::get_slot_tag<F>::type
      tag_type(const F&)
      {
        typedef typename signalslib::detail::get_slot_tag<F>::type
        the_tag_type;
        the_tag_type tag = the_tag_type();
        return tag;
      }
    }
  }
} // end namespace boost

#define BOOST_PP_ITERATION_LIMITS (0, BOOST_SIGNALS_MAX_ARGS)
#define BOOST_PP_FILENAME_1 <boost/thread_safe_signals/detail/slot_template.hpp>
#include BOOST_PP_ITERATE()

namespace boost
{
  template<typename Signature,
    typename SlotFunction = boost::function<Signature> >
  class slot: public signalslib::detail::slotN<function_traits<Signature>::arity,
    Signature, SlotFunction>::type
  {
  private:
    typedef typename signalslib::detail::slotN<boost::function_traits<Signature>::arity,
      Signature, SlotFunction>::type base_type;
  public:
    template<typename F>
    slot(const F& f): base_type(f)
    {}
    // bind syntactic sugar
// AN aN
#define BOOST_SLOT_BINDING_ARG_DECL(z, n, data) \
  BOOST_PP_CAT(A, n) BOOST_PP_CAT(a, n)
// template<typename F, typename A0, typename A1, ..., typename An-1> slot(...
#define BOOST_SLOT_BINDING_CONSTRUCTOR(z, n, data) \
    template<typename F, BOOST_PP_ENUM_PARAMS(n, typename A)> \
    slot(F f, BOOST_PP_ENUM(n, BOOST_SLOT_BINDING_ARG_DECL, ~)): \
      base_type(f, BOOST_PP_ENUM_PARAMS(n, a)) \
    {}
#define BOOST_SLOT_MAX_BINDING_ARGS 10
  BOOST_PP_REPEAT_FROM_TO(1, BOOST_SLOT_MAX_BINDING_ARGS, BOOST_SLOT_BINDING_CONSTRUCTOR, ~)
#undef BOOST_SLOT_MAX_BINDING_ARGS
#undef BOOST_SLOT_BINDING_ARG_DECL
#undef BOOST_SLOT_BINDING_CONSTRUCTOR
  };
}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SIGNALS_SLOT_HEADER
