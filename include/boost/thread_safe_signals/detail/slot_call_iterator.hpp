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

#include <boost/assert.hpp>
#include <boost/aligned_storage.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread_safe_signals/connection.hpp>
#include <boost/type_traits.hpp>

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
				cache(&c), lock_iter(end_in)
			{
			}
			slot_call_iterator_t(const slot_call_iterator_t &other): iter(other.iter),
			end(other.end), f(other.f), cache(other.cache), lock_iter(other.lock_iter),
			tracked_ptrs(other.tracked_ptrs)
			{
				if(other.lock)
					lock.reset(new(lock_pool) lock_type(lock_pool, (*iter)->mutex));
			}
			const slot_call_iterator_t& operator =(const slot_call_iterator_t &other)
			{
				if(this == &other) return;
				iter = other.iter;
				end = other.end;
				f = other.f;
				cache = other.cache;
				if(other.lock)
					lock.reset(new(lock_pool) lock_type(lock_pool, (*iter)->mutex));
				else
					lock.reset();
				lock_iter = other.lock_iter;
				tracked_ptrs = other.tracked_ptrs;
				return *this;
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
			class lock_type;
			// lock_memory_pool and the lock_type class exist to optimize our dynamic allocation of scoped_locks.
			// It makes a noticeable difference in benchmarks when invoking a signal in an inner loop with
			// an empty slot connected.  I tried using boost::object_pool<lock_type> but it was slower.
			class lock_memory_pool
			{
			public:
				lock_memory_pool():
					_use_first(false), _alloc_count(0)
				{
				}
				void * malloc()
				{
					BOOST_ASSERT(_alloc_count < _max_objects);
					_use_first = !_use_first;
					++_alloc_count;
					return &_chunks[_use_first];
				}
				void free(void *)
				{
					--_alloc_count;
				}
			private:
				static const unsigned _max_objects = 2;
				
				aligned_storage<sizeof(lock_type), alignment_of<lock_type>::value>
					_chunks[_max_objects];
				bool _use_first;
				unsigned _alloc_count;
			};
			class lock_type: public ConnectionBody::mutex_type::scoped_lock
			{
			public:
				lock_type(lock_memory_pool &pool, typename ConnectionBody::mutex_type &mutex):
					ConnectionBody::mutex_type::scoped_lock(mutex), _pool(pool)
				{}
				static void* operator new(unsigned num_bytes, lock_memory_pool &pool)
				{
					return pool.malloc();
				}
				static void operator delete(void *ptr)
				{
					if(ptr == 0) return;
					lock_type *typed_ptr = static_cast<lock_type*>(ptr);
					lock_memory_pool &pool = typed_ptr->_pool;
					pool.free(typed_ptr);
				}
			private:
				lock_memory_pool &_pool;
			};
			
			
			void lockNextCallable() const
			{
				if(iter == lock_iter)
				{
					return;
				}
				for(;iter != end; ++iter)
				{
					lock.reset(new(lock_pool) lock_type(lock_pool, (*iter)->mutex));
					lock_iter = iter;
					tracked_ptrs = (*iter)->nolock_grab_tracked_objects();
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
			optional<result_type>* cache;
			mutable lock_memory_pool lock_pool;
			mutable scoped_ptr<lock_type> lock;
			mutable Iterator lock_iter;
			mutable typename ConnectionBody::shared_ptrs_type tracked_ptrs;
		};
		} // end namespace detail
	} // end namespace BOOST_SIGNALS_NAMESPACE
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TS_SIGNALS_SLOT_CALL_ITERATOR
