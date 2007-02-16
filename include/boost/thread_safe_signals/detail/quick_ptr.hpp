#ifndef BOOST_QUICK_PTR_HPP_INCLUDED
#define BOOST_QUICK_PTR_HPP_INCLUDED

//
//  quick_ptr.hpp:  A simple smart ptr similar to shared_ptr except
//  without a built-in mutex (and with less features).  It is intended
//  for internal use in boost.signals, and should be faster than
//  shared_ptr.  It assumes the user will take care to appropriate
//  locking if used in a multi-threaded environment.
//
//  (C) Copyright Greg Colvin and Beman Dawes 1998, 1999.
//  Copyright (c) 2001, 2002, 2003 Peter Dimov
//  Copyright (c) Frank Mori Hess 2007
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/shared_ptr.htm for documentation.
//

#include <boost/assert.hpp>
#include <boost/checked_delete.hpp>

namespace boost
{
	namespace signalslib
	{
		namespace detail
		{
			template<typename T> struct quick_ptr_traits
			{
				typedef T & reference;
			};

			template<class T> class quick_ptr
			{
			private:
				typedef quick_ptr<T> this_type;
			public:
				typedef T element_type;
				typedef T value_type;
				typedef T * pointer;
				typedef typename quick_ptr_traits<T>::reference reference;

				quick_ptr(): px(0), pn(0)
				{
				}
				quick_ptr(T * p): px(p), pn(0)
				{
					if(px)
					{
						pn = new long(1);
					}
				}
				quick_ptr(const quick_ptr<T> &other): px(other.px), pn(other.pn)
				{
					if(pn) ++(*pn);
				}
				~quick_ptr()
				{
					if(pn == 0) return;
					--(*pn);
					if(*pn == 0)
					{
						delete pn;
						checked_delete(px);
					}
				}
				const quick_ptr<T>& operator=(const quick_ptr<T> &other)
				{
					if(this == &other) return *this;
					px = other.px;
					pn = other.pn;
					if(pn) ++(*pn);
					return *this;
				}
				void reset()
				{
					this_type().swap(*this);
				}
				void reset(T * p)
				{
					BOOST_ASSERT(p == 0 || p != px); // catch self-reset errors
					this_type(p).swap(*this);
				}
				reference operator* () const // never throws
				{
					BOOST_ASSERT(px != 0);
					return *px;
				}

				T * operator-> () const // never throws
				{
					BOOST_ASSERT(px != 0);
					return px;
				}

				T * get() const // never throws
				{
					return px;
				}

				// implicit conversion to "bool"

			#if defined(__SUNPRO_CC) && BOOST_WORKAROUND(__SUNPRO_CC, <= 0x530)

				operator bool () const
				{
					return px != 0;
				}

			#elif defined(__MWERKS__) && BOOST_WORKAROUND(__MWERKS__, BOOST_TESTED_AT(0x3003))
				typedef T * (this_type::*unspecified_bool_type)() const;

				operator unspecified_bool_type() const // never throws
				{
					return px == 0? 0: &this_type::get;
				}

			#else

				typedef T * this_type::*unspecified_bool_type;

				operator unspecified_bool_type() const // never throws
				{
					return px == 0? 0: &this_type::px;
				}

			#endif

				// operator! is redundant, but some compilers need it

				bool operator! () const // never throws
				{
					return px == 0;
				}

				long use_count() const // never throws
				{
					if(pn) return *pn;
					return 0;
				}

				void swap(quick_ptr<T> & other) // never throws
				{
					std::swap(px, other.px);
					std::swap(pn, other.pn);
				}

				T * px;                     // contained pointer
				mutable long *pn;    // reference counter
			};  // quick_ptr

			template<class T, class U> inline bool operator==(quick_ptr<T> const & a, quick_ptr<U> const & b)
			{
				return a.get() == b.get();
			}

			template<class T, class U> inline bool operator!=(quick_ptr<T> const & a, quick_ptr<U> const & b)
			{
				return a.get() != b.get();
			}

			#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96

			// Resolve the ambiguity between our op!= and the one in rel_ops

			template<class T> inline bool operator!=(quick_ptr<T> const & a, quick_ptr<T> const & b)
			{
				return a.get() != b.get();
			}

			#endif

			template<class T, class U> inline bool operator<(quick_ptr<T> const & a, quick_ptr<U> const & b)
			{
				return a.get() < b.get();
			}

			template<class T> inline void swap(quick_ptr<T> & a, quick_ptr<T> & b)
			{
				a.swap(b);
			}

			// get_pointer() enables boost::mem_fn to recognize quick_ptr

			template<class T> inline T * get_pointer(quick_ptr<T> const & p)
			{
				return p.get();
			}
		}
	}
} // namespace boost

#ifdef BOOST_MSVC
# pragma warning(pop)
#endif

#endif  // #ifndef BOOST_QUICK_PTR_HPP_INCLUDED
