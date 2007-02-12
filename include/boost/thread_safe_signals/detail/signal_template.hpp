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
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is included iteratively, and should not be protected from multiple inclusion

#define EPG_SIGNALS_NUM_ARGS BOOST_PP_ITERATION()

#define EPG_SIGNAL_CLASS_NAME BOOST_PP_CAT(signal, EPG_SIGNALS_NUM_ARGS)

// argn
#define EPG_SIGNAL_SIGNATURE_ARG_NAME(z, n, data) BOOST_PP_CAT(arg, BOOST_PP_INC(n))
// Tn argn
#define EPG_SIGNAL_SIGNATURE_FULL_ARG(z, n, data) \
	BOOST_PP_CAT(T, BOOST_PP_INC(n)) EPG_SIGNAL_SIGNATURE_ARG_NAME(~, n, ~)
// T1 arg1, T2 arg2, ...
// Tn argn
#define EPG_SIGNAL_SIGNATURE_FULL_ARGS(arity, data) \
	BOOST_PP_ENUM(arity, EPG_SIGNAL_SIGNATURE_FULL_ARG, data)
// arg1, arg2, ..., argn
#define EPG_SIGNAL_SIGNATURE_ARG_NAMES(arity) BOOST_PP_ENUM(arity, EPG_SIGNAL_SIGNATURE_ARG_NAME, ~)

namespace boost
{
	template<typename R, BOOST_PP_ENUM_SHIFTED_PARAMS(BOOST_PP_INC(EPG_SIGNALS_NUM_ARGS), typename T) BOOST_PP_COMMA_IF(EPG_SIGNALS_NUM_ARGS)
		typename Combiner = boost::last_value<R>,
		typename Group = int,
		typename GroupCompare = std::less<Group>,
		typename SlotFunction = BOOST_PP_CAT(boost::function, EPG_SIGNALS_NUM_ARGS)<R BOOST_PP_COMMA_IF(EPG_SIGNALS_NUM_ARGS)
			BOOST_PP_ENUM_SHIFTED_PARAMS(BOOST_PP_INC(EPG_SIGNALS_NUM_ARGS), T) > >
	class EPG_SIGNAL_CLASS_NAME
	{
	private:
		class slot_invoker;
		typedef typename signalslib::detail::group_key<Group>::type group_key_type;
		typedef boost::shared_ptr<signalslib::detail::ConnectionBody<group_key_type, SlotFunction> > connection_body_type;
		typedef std::list<connection_body_type> ConnectionList;
	public:
		typedef SlotFunction slot_function_type;
		typedef slot<slot_function_type> slot_type;
		typedef typename slot_function_type::result_type slot_result_type;
		typedef Combiner combiner_type;
		typedef typename combiner_type::result_type result_type;
		typedef Group group_type;
		typedef GroupCompare group_compare_type;
		typedef typename signalslib::detail::slot_call_iterator_t<slot_invoker,
			typename ConnectionList::iterator > slot_call_iterator;
// typedef Tn argn_type;
#define EPG_SIGNAL_MISC_STATEMENT(z, n, data) \
	typedef BOOST_PP_CAT(T, BOOST_PP_INC(n)) BOOST_PP_CAT(BOOST_PP_CAT(arg, BOOST_PP_INC(n)), _type);
		BOOST_PP_REPEAT(EPG_SIGNALS_NUM_ARGS, EPG_SIGNAL_MISC_STATEMENT, ~)
#undef EPG_SIGNAL_MISC_STATEMENT
#if EPG_SIGNALS_NUM_ARGS == 1
		typedef arg1_type argument_type;
#elif EPG_SIGNALS_NUM_ARGS == 2
		typedef arg1_type first_argument_type;
		typedef arg2_type second_argument_type;
#endif
		static const int arity = EPG_SIGNALS_NUM_ARGS;

		EPG_SIGNAL_CLASS_NAME(const combiner_type &combiner = combiner_type(),
			const group_compare_type &group_compare = group_compare_type()):
			_combiner(new combiner_type(combiner)), _connectionBodies(new ConnectionList),
			_group_key_comparator(group_compare)
		{};
		virtual ~EPG_SIGNAL_CLASS_NAME()
		{}
		// connect slot
		signalslib::connection connect(const slot_type &slot, signalslib::connect_position position = signalslib::at_back)
		{
			boost::mutex::scoped_lock lock(_mutex);
			connection_body_type newConnectionBody =
				create_new_connection(slot);
			group_key_type group_key;
			if(position == signalslib::at_back)
			{
				group_key.first = signalslib::detail::back_ungrouped_slots;
				_connectionBodies->push_back(newConnectionBody);
			}else
			{
				group_key.first = signalslib::detail::front_ungrouped_slots;
				_connectionBodies->push_front(newConnectionBody);
			}
			newConnectionBody->set_group_key(group_key);
			return signalslib::connection(newConnectionBody);
		}
		signalslib::connection connect(const group_type &group,
			const slot_type &slot, signalslib::connect_position position = signalslib::at_back)
		{
			boost::mutex::scoped_lock lock(_mutex);
			connection_body_type newConnectionBody =
				create_new_connection(slot);
			// update map to first connection body in group if needed
			group_key_type group_key(signalslib::detail::grouped_slots, group);
			newConnectionBody->set_group_key(group_key);
			if(position == signalslib::at_back)
			{
				typename ConnectionList::reverse_iterator rit;
				for(rit = _connectionBodies->rbegin(); rit != _connectionBodies->rend(); ++rit)
				{
					if(_group_key_comparator(group_key, (*rit)->group_key()) == false)
					{
						_connectionBodies->insert(rit.base(), newConnectionBody);
						break;
					}
				}
				if(rit == _connectionBodies->rend())
				{
					_connectionBodies->insert(rit.base(), newConnectionBody);
				}
			}else	// at_front
			{
				typename ConnectionList::iterator it;
				for(it = _connectionBodies->begin(); it != _connectionBodies->end(); ++it)
				{
					if(_group_key_comparator((*it)->group_key(), group_key) == false)
					{
						_connectionBodies->insert(it, newConnectionBody);
						break;
					}
				}
				if(it == _connectionBodies->end())
				{
					_connectionBodies->insert(it, newConnectionBody);
				}
			}
			return signalslib::connection(newConnectionBody);
		}
		// disconnect slot(s)
		void disconnect_all_slots()
		{
			boost::mutex::scoped_lock listLock(_mutex);
			typename ConnectionList::iterator it;
			for(it = _connectionBodies->begin(); it != _connectionBodies->end(); ++it)
			{
				(*it)->disconnect();
			}
			_connectionBodies->clear();
		}
		void disconnect(const group_type &group)
		{
			boost::mutex::scoped_lock listLock(_mutex);
			group_key_type group_key(signalslib::detail::grouped_slots, group);
			typename ConnectionList::iterator it;
			for(it = _connectionBodies->begin(); it != _connectionBodies->end();)
			{
				if(_group_key_comparator((*it)->group_key(), group_key))
				{
					++it;
					continue;
				}
				if(_group_key_comparator(group_key, (*it)->group_key())) break;
				(*it)->disconnect();
				it = _connectionBodies->erase(it);
			}
		}
		template <typename Slot>
		void disconnect(const Slot &slot)
		{
			boost::mutex::scoped_lock listLock(_mutex);
			typename ConnectionList::iterator it;
			for(it = _connectionBodies->begin(); it != _connectionBodies->end(); ++it)
			{
				bool disconnect;
				{
					typename signalslib::detail::ConnectionBodyBase::mutex_type::scoped_lock lock((*it)->mutex);
					if((*it)->slot == slot)
					{
						disconnect = true;
					}else
					{
						disconnect = false;
					}
				}// scoped_lock destructs here, safe to erase now
				if(disconnect)
				{
					(*it)->nolock_disconnect();
					it = _connectionBodies->erase(it);
					break;
				}
			}
		}
		// emit signal
		result_type operator ()(EPG_SIGNAL_SIGNATURE_FULL_ARGS(EPG_SIGNALS_NUM_ARGS, ~))
		{
			boost::shared_ptr<ConnectionList> localConnectionBodies;
			boost::shared_ptr<combiner_type> local_combiner;
			typename ConnectionList::iterator it;
			{
				boost::mutex::scoped_lock listLock(_mutex);
				// only clean up if it is safe to do so
				if(_connectionBodies.use_count() == 1)
					nolockCleanupConnections(_connectionBodies, false);
				localConnectionBodies = _connectionBodies;
				/* make a local copy of _combiner while holding mutex, so we are
				thread safe against the combiner getting modified by set_combiner()*/
				local_combiner = _combiner;
			}
			slot_invoker invoker;
// invoker.argn = argn;
#define EPG_SIGNAL_MISC_STATEMENT(z, n, data) \
	invoker.EPG_SIGNAL_SIGNATURE_ARG_NAME(~, n, ~) = EPG_SIGNAL_SIGNATURE_ARG_NAME(~, n, ~) ;
			BOOST_PP_REPEAT(EPG_SIGNALS_NUM_ARGS, EPG_SIGNAL_MISC_STATEMENT, ~)
#undef EPG_SIGNAL_MISC_STATEMENT
			boost::optional<typename signalslib::detail::slot_result_type_wrapper<slot_result_type>::type > cache;
			slot_call_iterator slot_iter_begin(
				localConnectionBodies->begin(), localConnectionBodies->end(), invoker, cache);
			slot_call_iterator slot_iter_end(
				localConnectionBodies->end(), localConnectionBodies->end(), invoker, cache);
			return (*local_combiner)(slot_iter_begin, slot_iter_end);
		}
		std::size_t num_slots() const
		{
			boost::mutex::scoped_lock listLock(_mutex);
			typename ConnectionList::iterator it;
			std::size_t count = 0;
			for(it = _connectionBodies->begin(); it != _connectionBodies->end(); ++it)
			{
				if((*it)->connected()) ++count;
			}
			return count;
		}
		bool empty() const
		{
			boost::mutex::scoped_lock listLock(_mutex);
			typename ConnectionList::iterator it;
			for(it = _connectionBodies->begin(); it != _connectionBodies->end(); ++it)
			{
				if((*it)->connected()) return false;
			}
			return true;
		}
		combiner_type combiner() const
		{
			boost::mutex::scoped_lock lock(_mutex);
			return *_combiner;
		}
		void set_combiner(const combiner_type &combiner)
		{
			boost::mutex::scoped_lock lock(_mutex);
			_combiner.reset(new combiner_type(combiner));
		}
	private:
		// slot_invoker is passed to slot_call_iterator_t to run slots
		class slot_invoker
		{
		public:
			typedef typename signalslib::detail::slot_result_type_wrapper<slot_result_type>::type result_type;

			result_type operator ()(const connection_body_type &connectionBody) const
			{
				result_type *resolver = 0;
				return m_invoke(connectionBody,
					EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS) BOOST_PP_COMMA_IF(EPG_SIGNALS_NUM_ARGS)
					resolver);
			}
// typename Tn argn;
#define EPG_SIGNAL_MISC_STATEMENT(z, n, Signature) \
	BOOST_PP_CAT(T, BOOST_PP_INC(n)) EPG_SIGNAL_SIGNATURE_ARG_NAME(~, n, ~);
			BOOST_PP_REPEAT(EPG_SIGNALS_NUM_ARGS, EPG_SIGNAL_MISC_STATEMENT, ~)
#undef EPG_SIGNAL_MISC_STATEMENT
		private:
			signalslib::detail::unusable m_invoke(const connection_body_type &connectionBody,
				const signalslib::detail::unusable *resolver) const
			{
				connectionBody->slot(EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS));
				return signalslib::detail::unusable();
			}
			result_type m_invoke(const connection_body_type &connectionBody, ...) const
			{
				return connectionBody->slot(EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS));
			}
		};

		// clean up disconnected connections
		void nolockCleanupConnections(boost::shared_ptr<ConnectionList> &connectionBodies, bool grab_tracked)
		{
			assert(connectionBodies.use_count() == 1);
			typename ConnectionList::iterator it;
			for(it = connectionBodies->begin(); it != connectionBodies->end();)
			{
				bool connected;
				{
					// skip over slots that are busy
					signalslib::detail::ConnectionBodyBase::mutex_type::scoped_try_lock lock((*it)->mutex);
					if(lock.locked() == false) continue;
					if(grab_tracked)
						(*it)->nolock_grab_tracked_objects();
					connected = (*it)->nolock_nograb_connected();
				}// scoped lock destructs here, safe to erase now
				if(connected == false)
				{
					it = connectionBodies->erase(it);
				}else
				{
					++it;
				}
			}
		}
		/* Make a new copy of the slot list if it is currently being iterated through
		by signal invocation. */
		void nolockForceUniqueConnectionList()
		{
			if(_connectionBodies.use_count() > 1)
			{
				boost::shared_ptr<ConnectionList> newList(new ConnectionList(*_connectionBodies));
				_connectionBodies = newList;
			}
		}
		connection_body_type create_new_connection(const slot_type &slot)
		{
			nolockForceUniqueConnectionList();
			nolockCleanupConnections(_connectionBodies, true);
			return connection_body_type(new signalslib::detail::ConnectionBody<group_key_type, slot_function_type>(slot));
		}
		boost::shared_ptr<combiner_type> _combiner;
		boost::shared_ptr<ConnectionList> _connectionBodies;
		// holds first connection body of each group
		signalslib::detail::group_key_less<Group, GroupCompare> _group_key_comparator;
		mutable boost::mutex _mutex;
	};
	namespace signalslib
	{
		namespace detail
		{
			template<unsigned arity, typename Signature, typename Combiner,
				typename Group, typename GroupCompare, typename SlotFunction> class signalN;
			// partial template specialization
			template<typename Signature, typename Combiner, typename Group,
				typename GroupCompare, typename SlotFunction>
			class signalN<EPG_SIGNALS_NUM_ARGS, Signature, Combiner, Group,
				GroupCompare, SlotFunction>
			{
			public:
				typedef EPG_SIGNAL_CLASS_NAME<typename boost::function_traits<Signature>::result_type,
// typename boost::function_traits<Signature>::argn_type ,
#define EPG_SIGNAL_MISC_STATEMENT(z, n, Signature) \
	BOOST_PP_CAT(BOOST_PP_CAT(typename boost::function_traits<Signature>::arg, BOOST_PP_INC(n)), _type) ,
					BOOST_PP_REPEAT(EPG_SIGNALS_NUM_ARGS, EPG_SIGNAL_MISC_STATEMENT, Signature)
#undef EPG_SIGNAL_MISC_STATEMENT
					Combiner, Group,
					GroupCompare, SlotFunction> type;
			};
		}
	}
}

#undef EPG_SIGNALS_NUM_ARGS
#undef EPG_SIGNAL_CLASS_NAME
#undef EPG_SIGNAL_SIGNATURE_ARG_NAME
#undef EPG_SIGNAL_SIGNATURE_FULL_ARG
#undef EPG_SIGNAL_SIGNATURE_FULL_ARGS
#undef EPG_SIGNAL_SIGNATURE_ARG_NAMES
