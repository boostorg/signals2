// Boost.Signals library

// Copyright Frank Mori Hess 2007.
// Copyright Timmo Stange 2007.
// Copyright Douglas Gregor 2001-2004. Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_TSS_SIGNALS_TRACKABLE_HPP
#define BOOST_TSS_SIGNALS_TRACKABLE_HPP

#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
	namespace signalslib {
		namespace detail
		{
			class tracked_objects_visitor;
		}
		class trackable {
		protected:
			trackable(): _tracked_ptr(new int) {}
			trackable(const trackable &): _tracked_ptr(new int) {}
			trackable& operator=(const trackable &)
			{
				return *this;
			}
			~trackable() {}
		private:
			friend class detail::tracked_objects_visitor;
			shared_ptr<void> get_shared_ptr() const
			{
				BOOST_ASSERT(_tracked_ptr);
				return _tracked_ptr;
			}

			shared_ptr<void> _tracked_ptr;
		};
	} // end namespace BOOST_SIGNALS_NAMESPACE
} // end namespace boost

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TSS_SIGNALS_TRACKABLE_HPP
