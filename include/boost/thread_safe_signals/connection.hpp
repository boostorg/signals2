/*
	A minimal thread-safe (partial) implementation of boost.signals.
	If you are just doing single-threaded code, use the
	real boost.signals instead.  This only exists because
	boost.signals isn't thread-safe. This only supports signals
	with a single parameter, and no return value.  There
	is no implementation of signals::trackable.  For automatic
	disconnection on object destruction, keep
	scoped_connections in objects that have member functions
	bound to signals,

	Author: Frank Hess <frank.hess@nist.gov>
	Begin: 2007-01-23
*/
/* This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to title 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and is
 * in the public domain. This is an experimental system. NIST assumes no
 * responsibility whatsoever for its use by other parties, and makes no
 * guarantees, expressed or implied, about its quality, reliability, or
 * any other characteristic. We would appreciate acknowledgement if the
 * software is used.
 */

#ifndef _EPG_SIGNALS_CONNECTION_H
#define _EPG_SIGNALS_CONNECTION_H

#include <boost/function.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread_safe_signals/track.hpp>
#include <boost/type_traits.hpp>
#include <boost/visit_each.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

namespace EPG
{
	namespace signalslib
	{
		namespace detail
		{
			class ConnectionBodyBase;

			// Visitor to collect tracked_base-derived objects from a bound function.
			class tracked_objects_visitor
			{
			public:
				tracked_objects_visitor(EPG::signalslib::detail::ConnectionBodyBase* slot) : slot_(slot)
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

				mutable EPG::signalslib::detail::ConnectionBodyBase* slot_;
			};

			class ConnectionBodyBase
			{
			public:
				typedef std::vector<boost::shared_ptr<void> > shared_ptrs_type;

				ConnectionBodyBase(): _connected(true)
				{}
				virtual ~ConnectionBodyBase() {}
				void disconnect()
				{
					boost::mutex::scoped_lock lock(mutex);
					if(_connected)
					{
						_connected = false;
					}
				}
				bool connected() const
				{
					boost::mutex::scoped_lock lock(mutex);
					grabTrackedObjects();
					return nolock_connected();
				}
				bool nolock_connected() const {return _connected;}
				void add_tracked(const boost::signalslib::detail::tracked_base &tracked)
				{
					_trackedObjects.push_back(tracked.get_shared_ptr());
				}
				// mutex should be locked when calling grabTrackedObjects
				shared_ptrs_type grabTrackedObjects() const
				{
					shared_ptrs_type sharedPtrs;
					tracked_objects_type::const_iterator it;
					for(it = _trackedObjects.begin(); it != _trackedObjects.end(); ++it)
					{
						sharedPtrs.push_back(it->lock());
						if(sharedPtrs.back() == 0)
						{
							_connected = false;
						}
					}
					return sharedPtrs;
				}
				mutable boost::mutex mutex;
			private:
				typedef std::vector<boost::weak_ptr<void> > tracked_objects_type;

				mutable bool _connected;
				tracked_objects_type _trackedObjects;
			};

			template<typename Signature>
			class ConnectionBody: public ConnectionBodyBase
			{
			public:
				ConnectionBody(const boost::function<Signature> &slotParameter):
					ConnectionBodyBase(), slot(slotParameter)
				{
				}
				virtual ~ConnectionBody() {}
				/* base class mutex should be locked and nolock_connected() checked
				before slot is called, to prevent races
				with connect() and disconnect() */
				const boost::function<Signature> slot;
			};
		}

		class connection
		{
		public:
			connection() {}
			connection(boost::weak_ptr<detail::ConnectionBodyBase> connectionBody):
				_weakConnectionBody(connectionBody)
			{}
			virtual ~connection() {}
			void disconnect() const
			{
				boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
				if(connectionBody == 0) return;
				connectionBody->disconnect();
			}
			bool connected() const
			{
				boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
				if(connectionBody == 0) return false;
				return connectionBody->connected();
			}
		private:
			boost::weak_ptr<detail::ConnectionBodyBase> _weakConnectionBody;
		};

		class scoped_connection: public EPG::signalslib::connection, boost::noncopyable
		{
		public:
			virtual ~scoped_connection()
			{
				disconnect();
			}
			const scoped_connection& operator=(const EPG::signalslib::connection &rhs)
			{
				EPG::signalslib::connection::operator=(rhs);
				return *this;
			}
		};
	}
}

void EPG::signalslib::detail::tracked_objects_visitor::maybe_add_tracked(const boost::signalslib::detail::tracked_base& t, boost::mpl::bool_<true>) const
{
	slot_->add_tracked(t);
}

#endif	// _EPG_SIGNALS_CONNECTION_H
