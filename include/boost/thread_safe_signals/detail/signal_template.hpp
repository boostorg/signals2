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
			template<typename Signature, typename Combiner, typename Group,
				typename GroupCompare, typename SlotFunction>
			class EPG_SIGNAL_CLASS_NAME
			{
			private:
				class slot_invoker;
				typedef group_key<Group>::type group_key_type;
				typedef slot_group_map<Group, GroupCompare>::map_type group_map_type;
				typedef boost::shared_ptr<ConnectionBody<Signature, group_key_type, SlotFunction> > connection_body_type;
				typedef std::list<connection_body_type> ConnectionList;
			public:
				typedef SlotFunction slot_function_type;
				typedef slot<slot_function_type> slot_type;
				typedef typename slot_function_type::result_type slot_result_type;
				typedef Combiner combiner_type;
				typedef typename combiner_type::result_type result_type;
				typedef Group group_type;
				typedef GroupCompare group_compare_type;
				typedef slot_call_iterator_t<slot_invoker, ConnectionList::iterator > slot_call_iterator;
// typedef typename boost::function_traits<Signature>::argn_type argn_type
#define EPG_SIGNAL_MISC_STATEMENT(z, n, Signature) \
	typedef EPG_SIGNAL_SIGNATURE_ARG_TYPE(~, n, Signature) BOOST_PP_CAT(BOOST_PP_CAT(arg, BOOST_PP_INC(n)), _type);
				BOOST_PP_REPEAT(EPG_SIGNALS_NUM_ARGS, EPG_SIGNAL_MISC_STATEMENT, Signature)
#undef EPG_SIGNAL_MISC_STATEMENT
#if EPG_SIGNALS_NUM_ARGS == 1
				typedef arg1_type argument_type;
#elif EPG_SIGNALS_NUM_ARGS == 2
				typedef arg1_type first_argument_type;
				typedef arg2_type second_argument_type;
#endif
				static const int arity = EPG_SIGNALS_NUM_ARGS;

				EPG_SIGNAL_CLASS_NAME(const combiner_type &combiner, const group_compare_type &group_compare):
                	_combiner(new combiner_type(combiner)), _connectionBodies(new ConnectionList),
					_group_key_comparator(group_compare)
				{};
				virtual ~EPG_SIGNAL_CLASS_NAME()
				{}
				// connect slot
				EPG::signalslib::connection connect(const slot_type &slot, signalslib::connect_position position = at_back)
				{
					boost::mutex::scoped_lock lock(_mutex);
					connection_body_type newConnectionBody =
						create_new_connection(slot);
					group_key_type group_key;
					if(position == at_back)
					{
						_connectionBodies->push_back(newConnectionBody);
						// update map to first connection body in back_ungrouped_slots if needed
						group_key.first = back_ungrouped_slots;
						if(_groupMap.find(group_key) == _groupMap.end())
						{
							_groupMap.insert(group_map_type::value_type(group_key, newConnectionBody));
						}
					}else
					{
						_connectionBodies->push_front(newConnectionBody);
						// update map to first connection body in front_ungrouped_slots
						group_key.first = front_ungrouped_slots;
						_groupMap.insert(group_map_type::value_type(group_key, newConnectionBody));
					}
					newConnectionBody->set_group_key(group_key);
					return EPG::signalslib::connection(newConnectionBody);
				}
				EPG::signalslib::connection connect(const group_type &group,
					const slot_type &slot, signalslib::connect_position position = at_back)
				{
					boost::mutex::scoped_lock lock(_mutex);
					connection_body_type newConnectionBody =
						create_new_connection(slot);
					ConnectionList::iterator connection_insert_it;
					// update map to first connection body in group if needed
					group_key_type group_key(grouped_slots,
						boost::shared_ptr<group_type>(new group_type(group)));
					newConnectionBody->set_group_key(group_key);
					group_map_type::iterator group_it;
					if(position == at_back)
					{
						group_it = _groupMap.upper_bound(group_key);
						connection_insert_it = std::find(_connectionBodies.begin(), _connectionBodies.end(), group_it->second);
						if(group_it == _groupMap.begin())
						{
							_groupMap.insert(group_map_type::value_type(group_key, newConnectionBody));
						}else
						{
							group_map_type::iterator previous = group_it;
							--previous;
							if(weakly_equivalent(_group_key_comparator, group_key, previous->first) == false)
							{
								_groupMap.insert(group_map_type::value_type(group_key, newConnectionBody));
							}
						}
					}else	// at_front
					{
						group_it = _groupMap.lower_bound(group_key);
						connection_insert_it = std::find(_connectionBodies.begin(), _connectionBodies.end(), group_it->second);
						_groupMap.insert(group_map_type::value_type(group_key, newConnectionBody));
						if(weakly_equivalent(_group_key_comparator, group_key, group_it->first))
						{
							_groupMap.erase(group_it);
						}
					}
					_connectionBodies->insert(connection_insert_it, newConnectionBody);
					return EPG::signalslib::connection(newConnectionBody);
				}
				// disconnect slot(s)
				void disconnect_all_slots()
				{
					boost::mutex::scoped_lock listLock(_mutex);
					ConnectionList::iterator it;
					for(it = _connectionBodies.begin(); it != _connectionBodies.end(); ++it)
					{
						(*it)->disconnect();
					}
					_connectionBodies.clear();
					_groupMap.clear();
				}
				void disconnect(const group_type &group)
				{
					boost::mutex::scoped_lock listLock(_mutex);
					group_key_type group_key(grouped_slots,
						boost::shared_ptr<group_type>(new group_type(group)));
					group_map_type::iterator group_begin_it =
						_groupMap.lower_bound(group_key);
					group_map_type::iterator group_end_it =
						_groupMap.upper_bound(group_key);
					if(group_begin_it == _groupMap.end())
						return;
					ConnectionList::iterator connnection_begin_it = group_begin_it->second;
					ConnectionList::iterator connnection_end_it;
					if(group_end_it == _groupMap.end())
					{
						connection_end_it = _connectionBodies.end();
					}else
					{
						connection_end_it = group_end_it->second;
					}
					ConnectionList::iterator it;
					for(it = _connection_begin_it; it != _connection_end_it; ++it)
					{
						(*it)->disconnect();
					}
					_connectionBodies.erase(_connection_begin_it, _connection_end_it);
					_groupMap.erase(group_begin_it, group_end_it);
				}
				// emit signal
				result_type operator ()(EPG_SIGNAL_SIGNATURE_FULL_ARGS(EPG_SIGNALS_NUM_ARGS, Signature))
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
#define EPG_SIGNAL_MISC_STATEMENT(z, n, Signature) \
	invoker.EPG_SIGNAL_SIGNATURE_ARG_NAME(~, n, ~) = EPG_SIGNAL_SIGNATURE_ARG_NAME(~, n, ~) ;
					BOOST_PP_REPEAT(EPG_SIGNALS_NUM_ARGS, EPG_SIGNAL_MISC_STATEMENT, ~)
#undef EPG_SIGNAL_MISC_STATEMENT
					boost::optional<slot_result_type_wrapper<slot_result_type>::type > cache;
					slot_call_iterator slot_iter_begin(
						localConnectionBodies->begin(), localConnectionBodies->end(), invoker, cache);
					slot_call_iterator slot_iter_end(
						localConnectionBodies->end(), localConnectionBodies->end(), invoker, cache);
					return (*local_combiner)(slot_iter_begin, slot_iter_end);
				}
				std::size_t num_slots() const
				{
					boost::mutex::scoped_lock listLock(_mutex);
					ConnectionList::iterator it;
					std::size_t count = 0;
					for(it = _connectionBodies.begin(); it != _connectionBodies.end(); ++it)
					{
						if((*it)->connected()) ++count;
					}
					return count;
				}
				bool empty() const
				{
					boost::mutex::scoped_lock listLock(_mutex);
					ConnectionList::iterator it;
					for(it = _connectionBodies.begin(); it != _connectionBodies.end(); ++it)
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
					typedef typename slot_result_type_wrapper<slot_result_type>::type result_type;

					result_type operator ()(const connection_body_type &connectionBody) const
					{
						result_type *resolver = 0;
						return m_invoke(connectionBody,
							EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS) BOOST_PP_COMMA_IF(EPG_SIGNALS_NUM_ARGS)
							resolver);
					}
// typename boost::function_traits<Signature>::argn_type argn;
#define EPG_SIGNAL_MISC_STATEMENT(z, n, Signature) \
	EPG_SIGNAL_SIGNATURE_ARG_TYPE(~, n, Signature) EPG_SIGNAL_SIGNATURE_ARG_NAME(~, n, ~);
					BOOST_PP_REPEAT(EPG_SIGNALS_NUM_ARGS, EPG_SIGNAL_MISC_STATEMENT, Signature)
#undef EPG_SIGNAL_MISC_STATEMENT
				private:
					unusable m_invoke(const connection_body_type &connectionBody, const unusable *resolver) const
					{
						connectionBody->slot(EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS));
						return unusable();
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
					ConnectionList::iterator it;
					for(it = connectionBodies->begin(); it != connectionBodies->end();)
					{
						// skip over slots that are busy
						ConnectionBodyBase::mutex_type::scoped_try_lock lock((*it)->mutex);
						if(lock.locked() == false) continue;
						if(grab_tracked)
							(*it)->nolock_grab_tracked_objects();
						if((*it)->nolock_nograb_connected() == false)
						{
							// update group map if needed
							group_map_type::iterator map_it =
								_groupMap.lower_bound((*it)->group_key());
							if(map_it != _groupMap.end())
							{
								// add new entry for first slot in group if needed
								ConnectionList::iterator next_connection_it = it;
								++next_connection_it;
								if(next_connection_it != connectionBodies->end() &&
									weakly_equivalent(_group_key_comparator, (*next_connection_it)->group_key(), (*it)->group_key()))
								{
									_groupMap.insert(group_map_type::value_type((*next_connection_it)->group_key(), *next_connection_it));
								}
								_groupMap.erase(map_it);
							}
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
					return connection_body_type(new ConnectionBody<Signature, group_key_type, slot_function_type>(slot));
				}
				boost::shared_ptr<combiner_type> _combiner;
				boost::shared_ptr<ConnectionList> _connectionBodies;
				// holds first connection body of each group
				group_map_type _groupMap;
				group_key_less<Group, GroupCompare> _group_key_comparator;
				mutable boost::mutex _mutex;
			};

			template<unsigned arity, typename Signature, typename Combiner,
				typename Group, typename GroupCompare, typename SlotFunction> class SignalN;
			// partial template specialization
			template<typename Signature, typename Combiner, typename Group,
				typename GroupCompare, typename SlotFunction>
			class SignalN<EPG_SIGNALS_NUM_ARGS, Signature, Combiner, Group,
				GroupCompare, SlotFunction>
			{
			public:
				typedef EPG_SIGNAL_CLASS_NAME<Signature, Combiner, Group,
					GroupCompare, SlotFunction> type;
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
