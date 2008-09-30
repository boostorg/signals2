/*
  A thread-safe version of Boost.Signals.

  Author: Frank Mori Hess <fmhess@users.sourceforge.net>
  Begin: 2007-01-23
*/
// Copyright Frank Mori Hess 2007-2008
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef _THREAD_SAFE_SIGNAL_HPP
#define _THREAD_SAFE_SIGNAL_HPP

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/function.hpp>
#include <boost/signals2/optional_last_value.hpp>
#include <boost/preprocessor/arithmetic.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/iteration.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2/detail/unique_lock.hpp>
#include <boost/type_traits.hpp>
#include <boost/signals2/detail/signals_common.hpp>
#include <boost/signals2/detail/signals_common_macros.hpp>
#include <boost/signals2/detail/slot_groups.hpp>
#include <boost/signals2/detail/slot_call_iterator.hpp>
#include <boost/signals2/mutex.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/shared_connection_block.hpp>
#include <boost/signals2/slot.hpp>
#include <functional>

#define BOOST_PP_ITERATION_LIMITS (0, BOOST_SIGNALS_MAX_ARGS)
#define BOOST_PP_FILENAME_1 <boost/signals2/detail/signal_template.hpp>
#include BOOST_PP_ITERATE()

namespace boost
{
  namespace signals2
  {
    template<typename Signature,
      typename Combiner = optional_last_value<typename boost::function_traits<Signature>::result_type>,
      typename Group = int,
      typename GroupCompare = std::less<Group>,
      typename SlotFunction = function<Signature>,
      typename Mutex = mutex >
    class signal: public detail::signalN<function_traits<Signature>::arity,
      Signature, Combiner, Group, GroupCompare, SlotFunction, Mutex>::type
    {
    private:
      typedef typename detail::signalN<boost::function_traits<Signature>::arity,
        Signature, Combiner, Group, GroupCompare, SlotFunction, Mutex>::type base_type;
    public:
      signal(const Combiner &combiner = Combiner(), const GroupCompare &group_compare = GroupCompare()):
        base_type(combiner, group_compare)
      {}
    };
  }
}

#endif	// _THREAD_SAFE_SIGNAL_HPP
