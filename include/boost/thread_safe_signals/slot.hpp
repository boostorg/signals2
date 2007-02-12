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

#include <boost/thread_safe_signals/track.hpp>
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
			class slot_base;
			template<typename GroupKey, typename SlotFunction>
			class ConnectionBody;

			// Visitor to collect tracked_base-derived objects from a bound function.
			class tracked_objects_visitor
			{
			public:
				tracked_objects_visitor(slot_base *slot) : slot_(slot)
				{}
				template<typename T>
				void operator()(const T& t) const {
					maybe_add_tracked(t, boost::mpl::bool_<boost::is_convertible<T*, boost::signalslib::detail::tracked_base*>::value>());
					{}
				}
				template<typename T>
				void operator()(boost::reference_wrapper<T> const & r) const {
					maybe_add_tracked(r.get(), boost::mpl::bool_<boost::is_convertible<T*, boost::signalslib::detail::tracked_base*>::value>());
					{}
				}
			private:
				inline void maybe_add_tracked(const boost::signalslib::detail::tracked_base& t, boost::mpl::bool_<true>) const;
				template<typename T>
				void maybe_add_tracked(const T&, boost::mpl::bool_<false>) const {}

				mutable slot_base * slot_;
			};

			class slot_base
			{
			public:
				friend class signalslib::detail::tracked_objects_visitor;
				template<typename GroupKey, typename SlotFunction>
					friend class ConnectionBody;
			private:
				typedef std::vector<boost::weak_ptr<void> > tracked_objects_container;

				void add_tracked(const boost::signalslib::detail::tracked_base &tracked)
				{
					_trackedObjects.push_back(tracked.get_shared_ptr());
				}
				const tracked_objects_container& get_all_tracked() const {return _trackedObjects;}

				tracked_objects_container _trackedObjects;
			};
		}
	}
	// slot class template.
	template<typename SlotFunction>
	class slot: public signalslib::detail::slot_base
	{
	public:
		template<typename F>
		slot(const F& f)
		: slot_function(f)
		{
		signalslib::detail::tracked_objects_visitor visitor(this);
		boost::visit_each(visitor, f, 0);
		}
		// We would have to enumerate all of the signalN classes here as friends
		// to make this private (as it otherwise should be). We can't name all of
		// them because we don't know how many there are.
	public:
		// Get the slot function to call the actual slot
		const SlotFunction& get_slot_function() const { return slot_function; }
	private:

		slot(); // no default constructor
		slot& operator=(const slot&); // no assignment operator

		SlotFunction slot_function;
	};
} // end namespace boost

void boost::signalslib::detail::tracked_objects_visitor::maybe_add_tracked(const boost::signalslib::detail::tracked_base& t, boost::mpl::bool_<true>) const
{
	slot_->add_tracked(t);
}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SIGNALS_SLOT_HEADER
