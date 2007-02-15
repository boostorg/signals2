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

#include <boost/mpl/bool.hpp>
#include <boost/ref.hpp>
#include <boost/thread_safe_signals/detail/signals_common.hpp>
#include <boost/thread_safe_signals/track.hpp>
#include <boost/thread_safe_signals/trackable.hpp>
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
			class slot_base;
			template<typename GroupKey, typename SlotFunction>
			class ConnectionBody;

			// Visitor to collect tracked objects from a bound function.
			class tracked_objects_visitor
			{
			public:
				tracked_objects_visitor(slot_base *slot) : slot_(slot)
				{}
				template<typename T>
				void operator()(const T& t) const
				{
					m_visit_reference_wrapper(t, mpl::bool_<is_reference_wrapper<T>::value>());
				}
			private:
				template<typename T>
				void m_visit_reference_wrapper(const reference_wrapper<T> &t, const mpl::bool_<true> &) const
				{
					m_visit_pointer(t.get_pointer(), mpl::bool_<true>());
				}
				template<typename T>
				void m_visit_reference_wrapper(const T &t, const mpl::bool_<false> &) const
				{
					m_visit_pointer(t, mpl::bool_<is_pointer<T>::value>());
				}
				template<typename T>
				void m_visit_pointer(const T &t, const mpl::bool_<true> &) const
				{
					m_visit_not_function_pointer(t, mpl::bool_<is_convertible<T, const void*>::value>());
				}
				template<typename T>
				void m_visit_pointer(const T &t, const mpl::bool_<false> &) const
				{
					m_visit_pointer(addressof(t), mpl::bool_<true>());
				}
				template<typename T>
				void m_visit_not_function_pointer(const T *t, const mpl::bool_<true> &) const
				{
					m_visit_signal(t, mpl::bool_<is_signal<T>::value>());
				}
				template<typename T>
				void m_visit_not_function_pointer(const T &t, const mpl::bool_<false> &) const
				{}
				template<typename T>
				void m_visit_signal(const T *t, const mpl::bool_<true> &) const;
				template<typename T>
				void m_visit_signal(const T &t, const mpl::bool_<false> &) const
				{
					add_if_trackable(t);
				}
				template<typename T>
				void add_if_trackable(const tracked<T> *t) const;
				void add_if_trackable(const trackable *trackable) const;
				void add_if_trackable(const void *trackable) const {}

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
			boost::visit_each(visitor, f);
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
void boost::signalslib::detail::tracked_objects_visitor::m_visit_signal(
	const T *signal, const mpl::bool_<true> &) const
{
	if(signal)
		slot_->add_tracked(signal->lock_pimpl());
};
template<typename T>
void boost::signalslib::detail::tracked_objects_visitor::add_if_trackable(const tracked<T> *t) const
{
	if(t)
		slot_->add_tracked(*t);
}
void boost::signalslib::detail::tracked_objects_visitor::add_if_trackable(const trackable *trackable) const
{
	if(trackable)
		slot_->add_tracked(trackable->get_shared_ptr());
};

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_SIGNALS_SLOT_HEADER
