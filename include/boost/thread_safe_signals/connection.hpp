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
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/type_traits.hpp>
#include <boost/weak_ptr.hpp>

namespace EPG
{
	namespace signalslib
	{
		namespace detail
		{
			class ConnectionBodyBase
			{
			public:
				ConnectionBodyBase(const boost::function<void (const ConnectionBodyBase*)> &disconnectCallback):
					_disconnectCallback(disconnectCallback), _connected(true)
				{}
				virtual ~ConnectionBodyBase() {}
				void disconnect()
				{
					bool wasConnected;
					{
						boost::mutex::scoped_lock lock(mutex);
						if((wasConnected = _connected))
						{
							_connected = false;
						}
					} // scoped_lock destructs here
					if(wasConnected)
					{
						_disconnectCallback(this);
					}
				}
				bool connected() const
				{
					boost::mutex::scoped_lock lock(mutex);
					return _connected;
				}
				bool nolock_connected() const {return _connected;}

				mutable boost::mutex mutex;
			private:
				const boost::function<void (const ConnectionBodyBase*)> _disconnectCallback;
				bool _connected;
			};

			template<typename Signature>
			class ConnectionBody: public ConnectionBodyBase
			{
			public:
				ConnectionBody(const boost::function<Signature> &slotParameter,
					const boost::function<void (const ConnectionBodyBase*)> &disconnectCallback):
					ConnectionBodyBase(disconnectCallback), slot(slotParameter)
				{}
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
				boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody);
				if(connectionBody == 0) return;
				connectionBody->disconnect();
			}
			bool connected() const
			{
				boost::shared_ptr<detail::ConnectionBodyBase> connectionBody(_weakConnectionBody);
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

#endif	// _EPG_SIGNALS_CONNECTION_H
