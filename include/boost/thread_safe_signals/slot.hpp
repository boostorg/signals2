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

#include <boost/ref.hpp>
#include <boost/thread_safe_signals/detail/signals_common.hpp>
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
				void operator()(const T& t) const
				{
					maybe_add_tracked(t, boost::mpl::bool_<boost::is_convertible<T*, boost::signalslib::detail::tracked_base*>::value>());
					maybe_add_tracked(t, boost::mpl::bool_<boost::is_convertible<T*, signal_base*>::value>());
				}
				template<typename T>
				void operator()(boost::reference_wrapper<T> const & r) const
				{
					maybe_add_tracked(r.get(), boost::mpl::bool_<boost::is_convertible<T*, boost::signalslib::detail::tracked_base*>::value>());
				}
			private:
				template<typename T>
				void maybe_add_tracked(const tracked<T> &t, boost::mpl::bool_<true>) const;
				template<typename T>
				inline void maybe_add_tracked(const T &signal, boost::mpl::bool_<true>) const;
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

				void add_tracked(const shared_ptr<void> &tracked)
				{
					_trackedObjects.push_back(tracked);
				}
				const tracked_objects_container& get_all_tracked() const {return _trackedObjects;}

				tracked_objects_container _trackedObjects;
			};

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
	// slot class template.
	template<typename SlotFunction>
	class slot: public signalslib::detail::slot_base
	{
	public:
		template<typename F>
		slot(const F& f): slot_function(signalslib::detail::get_invocable_slot(f, signalslib::detail::tag_type(f)))
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

template<typename T>
void boost::signalslib::detail::tracked_objects_visitor::maybe_add_tracked(const tracked<T> &t, boost::mpl::bool_<true>) const
{
	slot_->add_tracked(t.get_shared_ptr());
}
template<typename T>
void boost::signalslib::detail::tracked_objects_visitor::maybe_add_tracked(const T &signal, boost::mpl::bool_<true>) const
{
	slot_->add_tracked(signal.lock_pimpl());
};

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SIGNALS_SLOT_HEADER
