// Boost.Signals2 library

// Copyright Douglas Gregor 2001-2004.
// Copyright Frank Mori Hess 2007-2008.
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_TS_SIGNALS_SLOT_CALL_ITERATOR
#define BOOST_TS_SIGNALS_SLOT_CALL_ITERATOR

#include <boost/assert.hpp>
#include <boost/aligned_storage.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/slot_base.hpp>
#include <boost/type_traits.hpp>
#include <boost/weak_ptr.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
  namespace signals2 {
    namespace detail {
      // Generates a slot call iterator. Essentially, this is an iterator that:
      //   - skips over disconnected slots in the underlying list
      //   - calls the connected slots when dereferenced
      //   - caches the result of calling the slots
      template<typename Function, typename Iterator, typename ConnectionBody>
      class slot_call_iterator_t
        : public boost::iterator_facade<slot_call_iterator_t<Function, Iterator, ConnectionBody>,
        typename Function::result_type,
        boost::single_pass_traversal_tag,
        typename Function::result_type const&>
      {
        typedef boost::iterator_facade<slot_call_iterator_t<Function, Iterator, ConnectionBody>,
          typename Function::result_type,
          boost::single_pass_traversal_tag,
          typename Function::result_type const&>
        inherited;

        typedef typename Function::result_type result_type;

        friend class boost::iterator_core_access;

      public:
        slot_call_iterator_t(Iterator iter_in, Iterator end_in, Function f,
          boost::optional<result_type> &c):
          iter(iter_in), end(end_in), f(f),
          cache(&c), callable_iter(end_in)
        {
          lockNextCallable();
        }

        typename inherited::reference
        dereference() const
        {
          if (!(*cache)) {
            try
            {
              cache->reset(f(*iter));
            }
            catch(const expired_slot &)
            {
              (*iter)->disconnect();
              throw;
            }
          }
          return cache->get();
        }

        void increment()
        {
          ++iter;
          lockNextCallable();
          cache->reset();
        }

        bool equal(const slot_call_iterator_t& other) const
        {
          return iter == other.iter;
        }

      private:
        typedef typename ConnectionBody::mutex_type::scoped_lock lock_type;

        void lockNextCallable() const
        {
          if(iter == callable_iter)
          {
            return;
          }
          for(;iter != end; ++iter)
          {
            lock_type lock((*iter)->mutex);
            tracked_ptrs = (*iter)->nolock_grab_tracked_objects();
            if((*iter)->nolock_nograb_blocked() == false)
            {
              callable_iter = iter;
              break;
            }
          }
          if(iter == end)
          {
            callable_iter = end;
          }
        }

        mutable Iterator iter;
        Iterator end;
        Function f;
        optional<result_type>* cache;
        mutable Iterator callable_iter;
        mutable typename slot_base::locked_container_type tracked_ptrs;
      };
    } // end namespace detail
  } // end namespace BOOST_SIGNALS_NAMESPACE
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TS_SIGNALS_SLOT_CALL_ITERATOR
