// Boost.Signals library

// Copyright Douglas Gregor 2001-2004.
// Copyright Frank Mori Hess 2007.
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_TS_SIGNALS_SLOT_CALL_ITERATOR
#define BOOST_TS_SIGNALS_SLOT_CALL_ITERATOR

#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread_safe_signals/connection.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
  namespace signalslib {
    namespace detail {

      // Generates a slot call iterator. Essentially, this is an iterator that:
      //   - skips over disconnected slots in the underlying list
      //   - calls the connected slots when dereferenced
      //   - caches the result of calling the slots
      template<typename Function, typename Iterator>
      class slot_call_iterator_t
        : public boost::iterator_facade<slot_call_iterator_t<Function, Iterator>,
                                 typename Function::result_type,
                                 boost::single_pass_traversal_tag,
                                 typename Function::result_type const&>
      {
        typedef boost::iterator_facade<slot_call_iterator_t<Function, Iterator>,
                                typename Function::result_type,
                                boost::single_pass_traversal_tag,
                                typename Function::result_type const&>
          inherited;

        typedef typename Function::result_type result_type;

        friend class boost::iterator_core_access;

      public:
        slot_call_iterator_t(Iterator iter_in, Iterator end_in, Function f,
                           boost::optional<result_type> &c)
          : iter(iter_in), end(end_in), f(f), cache(&c), lock_iter(end_in)
        {
        }

        typename inherited::reference
        dereference() const
        {
          if (!(*cache)) {
            lockNextCallable();
            cache->reset(f(*iter));
          }

          return cache->get();
        }

        void increment()
        {
          lockNextCallable();
          ++iter;
          lockNextCallable();
          cache->reset();
        }

        bool equal(const slot_call_iterator_t& other) const
        {
          lockNextCallable();
          other.lockNextCallable();
          return iter == other.iter;
        }

      private:
        typedef ConnectionBodyBase::mutex_type::scoped_lock lock_type;

        void lockNextCallable() const
        {
          if(iter == lock_iter)
          {
            return;
          }
          for(;iter != end; ++iter)
          {
            lock.reset(new lock_type((*iter)->mutex));
            lock_iter = iter;
            trackedPtrs = (*iter)->nolock_grab_tracked_objects();
            if((*iter)->nolock_nograb_blocked() == false) break;
          }
          if(iter == end)
          {
            lock.reset();
            lock_iter = end;
          }
        }

        mutable Iterator iter;
        Iterator end;
        Function f;
        boost::optional<result_type>* cache;
        mutable boost::shared_ptr<lock_type> lock;
        mutable Iterator lock_iter;
        mutable ConnectionBodyBase::shared_ptrs_type trackedPtrs;
      };
    } // end namespace detail
  } // end namespace BOOST_SIGNALS_NAMESPACE
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TS_SIGNALS_SLOT_CALL_ITERATOR
