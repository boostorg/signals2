/*
	Template for Signa1, Signal2, ... classes that support signals
	with 1, 2, ... parameters

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

// This file is included iteratively, and should not be protected from multiple inclusion

#define EPG_SIGNALS_NUM_ARGS BOOST_PP_ITERATION()

#define EPG_SIGNAL_CLASS_NAME BOOST_PP_CAT(Signal, EPG_SIGNALS_NUM_ARGS)

// typename boost::function_traits<Signature>::argn_type
#define EPG_SIGNAL_SIGNATURE_ARG_TYPE(z, n, Signature) \
	BOOST_PP_CAT(BOOST_PP_CAT(typename boost::function_traits<Signature>::arg, BOOST_PP_INC(n)), _type)
// argn
#define EPG_SIGNAL_SIGNATURE_ARG_NAME(z, n, data) BOOST_PP_CAT(arg, BOOST_PP_INC(n))
// typename boost::function_traits<Signature>::argn_type argn
#define EPG_SIGNAL_SIGNATURE_FULL_ARG(z, n, Signature) \
	EPG_SIGNAL_SIGNATURE_ARG_TYPE(~, n, Signature) EPG_SIGNAL_SIGNATURE_ARG_NAME(~, n, ~)
// typename boost::function_traits<Signature>::arg1_type arg1, typename boost::function_traits<Signature>::arg2_type arg2, ...
// typename boost::function_traits<Signature>::argn_type argn
#define EPG_SIGNAL_SIGNATURE_FULL_ARGS(arity, Signature) \
	BOOST_PP_ENUM(arity, EPG_SIGNAL_SIGNATURE_FULL_ARG, Signature)
// arg1, arg2, ..., argn
#define EPG_SIGNAL_SIGNATURE_ARG_NAMES(arity) BOOST_PP_ENUM(arity, EPG_SIGNAL_SIGNATURE_ARG_NAME, ~)

namespace EPG
{
	namespace signalslib
	{
		namespace detail
		{
			template<typename Signature>
			class EPG_SIGNAL_CLASS_NAME
			{
			public:
				typedef boost::function<Signature> slot_type;
				typedef void result_type;

				EPG_SIGNAL_CLASS_NAME() {};
				virtual ~EPG_SIGNAL_CLASS_NAME()
				{}
				// connect slot
				template<typename SlotType>
				EPG::signalslib::connection connect(const SlotType &slot)
				{
					boost::mutex::scoped_lock lock(_mutex);
					// clean up disconnected connections
					ConnectionList::iterator it;
					for(it = _connectionBodies.begin(); it != _connectionBodies.end();)
					{
						if((*it)->connected() == false)
						{
							it = _connectionBodies.erase(it);
						}else
						{
							++it;
						}
					}
					boost::shared_ptr<ConnectionBody<Signature> > newConnectionBody(new ConnectionBody<Signature>(slot));
					tracked_objects_visitor visitor(newConnectionBody.get());
					boost::visit_each(visitor, slot, 0);
					_connectionBodies.push_back(newConnectionBody);
					return EPG::signalslib::connection(_connectionBodies.back());
				}
				// emit signal
				result_type operator ()(EPG_SIGNAL_SIGNATURE_FULL_ARGS(EPG_SIGNALS_NUM_ARGS, Signature))
				{
					boost::mutex::scoped_lock listLock(_mutex);
					typename ConnectionList::iterator it;
					for(it = _connectionBodies.begin(); it != _connectionBodies.end();)
					{
						bool slotDisconnected;
						{
							boost::mutex::scoped_lock connectionLock((*it)->mutex);
							ConnectionBodyBase::shared_ptrs_type trackedPtrs = (*it)->grabTrackedObjects();
							if((*it)->nolock_connected())
							{
								(*it)->slot(EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS));
								slotDisconnected = false;
							}else
							{
								slotDisconnected = true;
							}
						}
						/* We have waited to erase the disconnected connection
						until after the connectionLock has gone out of scope to prevent
						connectionLock from manipulating its mutex after the
						ConnectionBody that contains the mutex has destructed */
						if(slotDisconnected)
						{
							/* erase() returns iterator to next unerased element, or end().  We
							use the return value to avoid being stuck with an invalid iterator */
							it = _connectionBodies.erase(it);
						}else
						{
							++it;
						}
					}
				}
			private:
				typedef std::list<boost::shared_ptr<ConnectionBody<Signature> > > ConnectionList;

				ConnectionList _connectionBodies;
				mutable boost::mutex _mutex;
			};

			template<unsigned arity, typename Signature> class SignalN;
			// partial template specialization
			template<typename Signature>
			class SignalN<EPG_SIGNALS_NUM_ARGS, Signature>
			{
			public:
				typedef EPG_SIGNAL_CLASS_NAME<Signature> type;
			};
		}
	}
}

#undef EPG_SIGNALS_NUM_ARGS
#undef EPG_SIGNAL_CLASS_NAME
#undef EPG_SIGNAL_SIGNATURE_ARG_TYPE
#undef EPG_SIGNAL_SIGNATURE_ARG_NAME
#undef EPG_SIGNAL_SIGNATURE_FULL_ARG
#undef EPG_SIGNAL_SIGNATURE_FULL_ARGS
#undef EPG_SIGNAL_SIGNATURE_ARG_NAMES
