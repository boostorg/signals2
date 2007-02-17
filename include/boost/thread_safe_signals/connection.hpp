/*

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
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _EPG_SIGNALS_CONNECTION_H
#define _EPG_SIGNALS_CONNECTION_H

#include <boost/function.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread_safe_signals/slot.hpp>
#include <boost/thread_safe_signals/track.hpp>
#include <boost/type_traits.hpp>
#include <boost/visit_each.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

namespace boost
{
	namespace signalslib
	{
		namespace detail
		{
			class ConnectionBodyBase
			{
			public:
				typedef std::vector<boost::shared_ptr<void> > shared_ptrs_type;
				typedef std::vector<boost::weak_ptr<void> > tracked_objects_container;

				ConnectionBodyBase(const tracked_objects_container &tracked_objects):
					_tracked_objects(tracked_objects), _connected(true), _blocked(false)
				{
				}
				virtual ~ConnectionBodyBase() {}
				virtual void disconnect() = 0;
				void nolock_disconnect()
				{
					if(_connected)
					{
						_connected = false;
					}
				}
				virtual bool connected() const = 0;
				virtual void block(bool should_block) = 0;
				virtual bool blocked() const = 0;
				bool nolock_nograb_blocked() const
				{
					return _blocked || (nolock_nograb_connected() == false);
				}
				bool nolock_nograb_connected() const {return _connected;}
				// mutex should be locked when calling grabTrackedObjects
				shared_ptrs_type nolock_grab_tracked_objects() const
				{
					shared_ptrs_type sharedPtrs;
					tracked_objects_container::const_iterator it;
					for(it = _tracked_objects.begin(); it != _tracked_objects.end(); ++it)
					{
						sharedPtrs.push_back(it->lock());
						if(sharedPtrs.back() == 0)
						{
							_connected = false;
							return shared_ptrs_type();
						}
					}
					return sharedPtrs;
				}
			protected:
				tracked_objects_container _tracked_objects;
				mutable bool _connected;
				bool _blocked;
			};

			template<typename GroupKey, typename SlotFunction, typename ThreadingModel>
			class ConnectionBody: public ConnectionBodyBase
			{
			public:
				typedef typename ThreadingModel::recursive_try_mutex_type mutex_type;
				ConnectionBody(const slot<SlotFunction> &slot_in):
					ConnectionBodyBase(slot_in.get_all_tracked()), slot(slot_in.get_slot_function())
				{
				}
				virtual ~ConnectionBody() {}
				virtual void disconnect()
				{
					typename mutex_type::scoped_lock lock(mutex);
					nolock_disconnect();
				}
				virtual bool connected() const
				{
					typename mutex_type::scoped_lock lock(mutex);
					nolock_grab_tracked_objects();
					return nolock_nograb_connected();
				}
				virtual void block(bool should_block)
				{
					typename mutex_type::scoped_lock lock(mutex);
					_blocked = should_block;
				}
				virtual bool blocked() const
				{
					typename mutex_type::scoped_lock lock(mutex);
					nolock_grab_tracked_objects();
					return nolock_nograb_blocked();
				}
				const GroupKey& group_key() const {return _group_key;}
				void set_group_key(const GroupKey &key) {_group_key = key;}
				/* base class mutex should be locked and nolock_nograb_blocked() checked
				before slot is called, to prevent races
				with connect() and disconnect() */
				const SlotFunction slot;
				mutable mutex_type mutex;
			private:
				GroupKey _group_key;
			};
		}

		class connection
		{
		public:
			connection() {}
			connection(const connection &other): _weakConnectionBody(other._weakConnectionBody)
			{}
			connection(boost::weak_ptr<detail::ConnectionBodyBase> connectionBody):
				_weakConnectionBody(connectionBody)
			{}
			~connection() {}
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
			void block(bool should_block=true)
			{
				boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
				if(connectionBody == 0) return;
				connectionBody->block(should_block);
			}
			void unblock()
			{
				block(false);
			}
			bool blocked() const
			{
				boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
				if(connectionBody == 0) return true;
				return connectionBody->blocked();
			}
			bool operator==(const connection& other) const
			{
				boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
				boost::shared_ptr<detail::ConnectionBodyBase> otherConnectionBody(other._weakConnectionBody.lock());
				return connectionBody == otherConnectionBody;
			}
			bool operator<(const connection& other) const
			{
				boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody.lock());
				boost::shared_ptr<detail::ConnectionBodyBase> otherConnectionBody(other._weakConnectionBody.lock());
				return connectionBody < otherConnectionBody;
			}
			void swap(connection &other)
			{
				std::swap(_weakConnectionBody, other._weakConnectionBody);
			}
		private:
			boost::weak_ptr<detail::ConnectionBodyBase> _weakConnectionBody;
		};

		class scoped_connection: public connection
		{
		public:
			scoped_connection() {}
			scoped_connection(const connection &other): connection(other)
			{}
			~scoped_connection()
			{
				disconnect();
			}
			const scoped_connection& operator=(const connection &rhs)
			{
				boost::signalslib::connection::operator=(rhs);
				return *this;
			}
			void swap(scoped_connection &other)
			{
				connection::swap(other);
			}
		};
	}
}

#endif	// _EPG_SIGNALS_CONNECTION_H
