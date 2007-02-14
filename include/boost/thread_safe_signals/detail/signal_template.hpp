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
#define EPG_WEAK_SIGNAL_CLASS_NAME BOOST_PP_CAT(weak_, EPG_SIGNAL_CLASS_NAME)
#define EPG_SIGNAL_IMPL_CLASS_NAME BOOST_PP_CAT(EPG_SIGNAL_CLASS_NAME, _impl)

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
// typename R, typename T1, typename T2, ..., typename TN, typename Combiner = boost::last_value<R>, ...
#define EPG_SIGNAL_TEMPLATE_DEFAULTED_DECL \
	typename R, BOOST_PP_ENUM_SHIFTED_PARAMS(BOOST_PP_INC(EPG_SIGNALS_NUM_ARGS), typename T) BOOST_PP_COMMA_IF(EPG_SIGNALS_NUM_ARGS) \
	typename Combiner = boost::last_value<R>, \
	typename Group = int, \
	typename GroupCompare = std::less<Group>, \
	typename SlotFunction = BOOST_PP_CAT(boost::function, EPG_SIGNALS_NUM_ARGS)<R BOOST_PP_COMMA_IF(EPG_SIGNALS_NUM_ARGS) \
		BOOST_PP_ENUM_SHIFTED_PARAMS(BOOST_PP_INC(EPG_SIGNALS_NUM_ARGS), T) >
// typename R, typename T1, typename T2, ..., typename TN, typename Combiner, ...
#define EPG_SIGNAL_TEMPLATE_DECL \
	typename R, BOOST_PP_ENUM_SHIFTED_PARAMS(BOOST_PP_INC(EPG_SIGNALS_NUM_ARGS), typename T) BOOST_PP_COMMA_IF(EPG_SIGNALS_NUM_ARGS) \
	typename Combiner, \
	typename Group, \
	typename GroupCompare, \
	typename SlotFunction
// R, T1, T2, ..., TN, Combiner, Group, GroupCompare, SlotFunction
#define EPG_SIGNAL_TEMPLATE_INSTANTIATION \
	R, BOOST_PP_ENUM_SHIFTED_PARAMS(BOOST_PP_INC(EPG_SIGNALS_NUM_ARGS), T) BOOST_PP_COMMA_IF(EPG_SIGNALS_NUM_ARGS) \
	Combiner, Group, GroupCompare, SlotFunction

namespace boost
{
	namespace signalslib
	{
		namespace detail
		{
			template<EPG_SIGNAL_TEMPLATE_DECL>
			class EPG_SIGNAL_IMPL_CLASS_NAME
			{
			private:
				class slot_invoker;
				typedef typename signalslib::detail::group_key<Group>::type group_key_type;
				typedef boost::shared_ptr<signalslib::detail::ConnectionBody<group_key_type, SlotFunction> > connection_body_type;
				typedef signalslib::detail::grouped_list<Group, GroupCompare, connection_body_type> connection_list_type;
			public:
				typedef SlotFunction slot_function_type;
				typedef slot<slot_function_type> slot_type;
				typedef typename slot_function_type::result_type slot_result_type;
				typedef Combiner combiner_type;
				typedef typename combiner_type::result_type result_type;
				typedef Group group_type;
				typedef GroupCompare group_compare_type;
				typedef typename signalslib::detail::slot_call_iterator_t<slot_invoker,
					typename connection_list_type::iterator > slot_call_iterator;

				EPG_SIGNAL_IMPL_CLASS_NAME(const combiner_type &combiner,
					const group_compare_type &group_compare):
					_combiner(new combiner_type(combiner)), _connectionBodies(new connection_list_type(group_compare))
				{};
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
						_connectionBodies->push_back(group_key, newConnectionBody);
					}else
					{
						group_key.first = signalslib::detail::front_ungrouped_slots;
						_connectionBodies->push_front(group_key, newConnectionBody);
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
						_connectionBodies->push_back(group_key, newConnectionBody);
					}else	// at_front
					{
						_connectionBodies->push_front(group_key, newConnectionBody);
					}
					return signalslib::connection(newConnectionBody);
				}
				// disconnect slot(s)
				void disconnect_all_slots()
				{
					shared_ptr<connection_list_type> connectionBodies =
						get_readable_connection_list();
					typename connection_list_type::iterator it;
					for(it = connectionBodies->begin(); it != connectionBodies->end(); ++it)
					{
						(*it)->disconnect();
					}
				}
				void disconnect(const group_type &group)
				{
					shared_ptr<connection_list_type> connectionBodies =
						get_readable_connection_list();
					group_key_type group_key(signalslib::detail::grouped_slots, group);
					typename connection_list_type::iterator it;
					typename connection_list_type::iterator end_it = connectionBodies->upper_bound(group_key);
					for(it = connectionBodies->lower_bound(group_key); it != end_it; ++it)
					{
						(*it)->disconnect();
					}
				}
				template <typename T>
				void disconnect(const T &slot)
				{
					typedef mpl::bool_<(is_convertible<T, group_type>::value)> is_group;
					do_disconnect(slot, is_group());
				}
				// emit signal
				result_type operator ()(EPG_SIGNAL_SIGNATURE_FULL_ARGS(EPG_SIGNALS_NUM_ARGS, ~))
				{
					boost::shared_ptr<connection_list_type> localConnectionBodies;
					boost::shared_ptr<combiner_type> local_combiner;
					typename connection_list_type::iterator it;
					{
						boost::mutex::scoped_lock listLock(_mutex);
						// only clean up if it is safe to do so
						if(_connectionBodies.use_count() == 1)
							nolockCleanupConnections(false);
						localConnectionBodies = _connectionBodies;
						/* make a local copy of _combiner while holding mutex, so we are
						thread safe against the combiner getting modified by set_combiner()*/
						local_combiner = _combiner;
					}
					slot_invoker invoker BOOST_PP_IF(EPG_SIGNALS_NUM_ARGS, \
						(EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS)), );
					boost::optional<typename signalslib::detail::slot_result_type_wrapper<slot_result_type>::type > cache;
					slot_call_iterator slot_iter_begin(
						localConnectionBodies->begin(), localConnectionBodies->end(), invoker, cache);
					slot_call_iterator slot_iter_end(
						localConnectionBodies->end(), localConnectionBodies->end(), invoker, cache);
					return (*local_combiner)(slot_iter_begin, slot_iter_end);
				}
				result_type operator ()(EPG_SIGNAL_SIGNATURE_FULL_ARGS(EPG_SIGNALS_NUM_ARGS, ~)) const
				{
					boost::shared_ptr<connection_list_type> localConnectionBodies;
					boost::shared_ptr<const combiner_type> local_combiner;
					typename connection_list_type::iterator it;
					{
						boost::mutex::scoped_lock listLock(_mutex);
						// only clean up if it is safe to do so
						if(_connectionBodies.use_count() == 1)
							nolockCleanupConnections(false);
						localConnectionBodies = _connectionBodies;
						/* make a local copy of _combiner while holding mutex, so we are
						thread safe against the combiner getting modified by set_combiner()*/
						local_combiner = _combiner;
					}
					slot_invoker invoker BOOST_PP_IF(EPG_SIGNALS_NUM_ARGS, \
						(EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS)), );
					boost::optional<typename signalslib::detail::slot_result_type_wrapper<slot_result_type>::type > cache;
					slot_call_iterator slot_iter_begin(
						localConnectionBodies->begin(), localConnectionBodies->end(), invoker, cache);
					slot_call_iterator slot_iter_end(
						localConnectionBodies->end(), localConnectionBodies->end(), invoker, cache);
					return (*local_combiner)(slot_iter_begin, slot_iter_end);
				}
				std::size_t num_slots() const
				{
					shared_ptr<connection_list_type> connectionBodies =
						get_readable_connection_list();
					typename connection_list_type::iterator it;
					std::size_t count = 0;
					for(it = connectionBodies->begin(); it != connectionBodies->end(); ++it)
					{
						if((*it)->connected()) ++count;
					}
					return count;
				}
				bool empty() const
				{
					shared_ptr<connection_list_type> connectionBodies =
						get_readable_connection_list();
					typename connection_list_type::iterator it;
					for(it = connectionBodies->begin(); it != connectionBodies->end(); ++it)
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

					slot_invoker(EPG_SIGNAL_SIGNATURE_FULL_ARGS(EPG_SIGNALS_NUM_ARGS, ~)) BOOST_PP_IF(EPG_SIGNALS_NUM_ARGS, :, )
// argn ( argn ) ,
#define EPG_SIGNAL_MISC_STATEMENT(z, n, data) \
	BOOST_PP_CAT(arg, n) ( BOOST_PP_CAT(arg, n) )
// arg1(arg1), arg2(arg2), ..., argn(argn)
						BOOST_PP_ENUM_SHIFTED(BOOST_PP_INC(EPG_SIGNALS_NUM_ARGS), EPG_SIGNAL_MISC_STATEMENT, ~)
#undef EPG_SIGNAL_MISC_STATEMENT
					{}
					result_type operator ()(const connection_body_type &connectionBody) const
					{
						result_type *resolver = 0;
						return m_invoke(connectionBody,
							resolver);
					}
// Tn argn;
#define EPG_SIGNAL_MISC_STATEMENT(z, n, Signature) \
	BOOST_PP_CAT(T, BOOST_PP_INC(n)) EPG_SIGNAL_SIGNATURE_ARG_NAME(~, n, ~);
					BOOST_PP_REPEAT(EPG_SIGNALS_NUM_ARGS, EPG_SIGNAL_MISC_STATEMENT, ~)
#undef EPG_SIGNAL_MISC_STATEMENT
				private:
					result_type m_invoke(const connection_body_type &connectionBody,
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
				void nolockCleanupConnections(bool grab_tracked) const
				{
					assert(_connectionBodies.use_count() == 1);
					typename connection_list_type::iterator it;
					for(it = _connectionBodies->begin(); it != _connectionBodies->end();)
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
							it = _connectionBodies->erase((*it)->group_key(), it);
						}else
						{
							++it;
						}
					}
				}
				/* Make a new copy of the slot list if it is currently being read somewhere else
				*/
				void nolock_force_unique_connection_list()
				{
					if(_connectionBodies.use_count() > 1)
					{
						boost::shared_ptr<connection_list_type> newList(new connection_list_type(*_connectionBodies));
						_connectionBodies = newList;
					}
				}
				shared_ptr<connection_list_type> get_readable_connection_list() const
				{
					boost::mutex::scoped_lock listLock(_mutex);
					return _connectionBodies;
				}
				connection_body_type create_new_connection(const slot_type &slot)
				{
					nolock_force_unique_connection_list();
					nolockCleanupConnections(true);
					return connection_body_type(new signalslib::detail::ConnectionBody<group_key_type, slot_function_type>(slot));
				}
				void do_disconnect(const group_type &group, mpl::bool_<true> is_group)
				{
					disconnect(group);
				}
				template<typename T>
				void do_disconnect(const T &slot, mpl::bool_<false> is_group)
				{
					shared_ptr<connection_list_type> connectionBodies =
						get_readable_connection_list();
					typename connection_list_type::iterator it;
					for(it = connectionBodies->begin(); it != connectionBodies->end(); ++it)
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
						}
					}
				}

				boost::shared_ptr<combiner_type> _combiner;
				boost::shared_ptr<connection_list_type> _connectionBodies;
				// connection list mutex must never be locked when attempting a blocking lock on a slot,
				// or you could deadlock.
				mutable boost::mutex _mutex;
			};

			template<EPG_SIGNAL_TEMPLATE_DECL>
			class EPG_WEAK_SIGNAL_CLASS_NAME;
		}
	}

	template<EPG_SIGNAL_TEMPLATE_DEFAULTED_DECL>
	class EPG_SIGNAL_CLASS_NAME: public signalslib::detail::signal_base
	{
	public:
		typedef signalslib::detail::EPG_WEAK_SIGNAL_CLASS_NAME<EPG_SIGNAL_TEMPLATE_INSTANTIATION> weak_signal_type;
		friend class weak_signal_type;
		friend class signalslib::detail::tracked_objects_visitor;

		typedef SlotFunction slot_function_type;
		typedef slot<slot_function_type> slot_type;
		typedef typename slot_function_type::result_type slot_result_type;
		typedef Combiner combiner_type;
		typedef typename combiner_type::result_type result_type;
		typedef Group group_type;
		typedef GroupCompare group_compare_type;
		typedef signalslib::detail::EPG_SIGNAL_IMPL_CLASS_NAME<EPG_SIGNAL_TEMPLATE_INSTANTIATION>::slot_call_iterator
			slot_call_iterator;
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
			_pimpl(new signalslib::detail::EPG_SIGNAL_IMPL_CLASS_NAME<EPG_SIGNAL_TEMPLATE_INSTANTIATION>(combiner, group_compare))
		{};
		~EPG_SIGNAL_CLASS_NAME()
		{
			disconnect_all_slots();
		}
		signalslib::connection connect(const slot_type &slot, signalslib::connect_position position = signalslib::at_back)
		{
			return (*_pimpl).connect(slot, position);
		}
		signalslib::connection connect(const group_type &group,
			const slot_type &slot, signalslib::connect_position position = signalslib::at_back)
		{
			return (*_pimpl).connect(group, slot, position);
		}
		void disconnect_all_slots()
		{
			(*_pimpl).disconnect_all_slots();
		}
		void disconnect(const group_type &group)
		{
			(*_pimpl).disconnect(group);
		}
		template <typename T>
		void disconnect(const T &slot)
		{
			(*_pimpl).disconnect(slot);
		}
		result_type operator ()(EPG_SIGNAL_SIGNATURE_FULL_ARGS(EPG_SIGNALS_NUM_ARGS, ~))
		{
			return (*_pimpl)(EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS));
		}
		result_type operator ()(EPG_SIGNAL_SIGNATURE_FULL_ARGS(EPG_SIGNALS_NUM_ARGS, ~)) const
		{
			return (*_pimpl)(EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS));
		}
		std::size_t num_slots() const
		{
			return (*_pimpl).num_slots();
		}
		bool empty() const
		{
			return (*_pimpl).empty();
		}
		combiner_type combiner() const
		{
			return (*_pimpl).combiner();
		}
		void set_combiner(const combiner_type &combiner)
		{
			return (*_pimpl).set_combiner(combiner);
		}
	private:
		shared_ptr<void> lock_pimpl() const
		{
			return _pimpl;
		}

		boost::shared_ptr<signalslib::detail::EPG_SIGNAL_IMPL_CLASS_NAME<EPG_SIGNAL_TEMPLATE_INSTANTIATION> >
			_pimpl;
	};

	namespace signalslib
	{
		namespace detail
		{
			// wrapper class for storing other signals as slots with automatic lifetime tracking
			template<EPG_SIGNAL_TEMPLATE_DECL>
			class EPG_WEAK_SIGNAL_CLASS_NAME
			{
			public:
				typedef SlotFunction slot_function_type;
				typedef typename slot_function_type::result_type slot_result_type;
				typedef EPG_SIGNAL_CLASS_NAME<EPG_SIGNAL_TEMPLATE_INSTANTIATION>::result_type
					result_type;

				EPG_WEAK_SIGNAL_CLASS_NAME(const EPG_SIGNAL_CLASS_NAME<EPG_SIGNAL_TEMPLATE_INSTANTIATION>
					&signal):
					_weak_pimpl(signal._pimpl)
				{}
				result_type operator ()(EPG_SIGNAL_SIGNATURE_FULL_ARGS(EPG_SIGNALS_NUM_ARGS, ~))
				{
					boost::shared_ptr<signalslib::detail::EPG_SIGNAL_IMPL_CLASS_NAME<EPG_SIGNAL_TEMPLATE_INSTANTIATION> >
						shared_pimpl(_weak_pimpl);
					return (*shared_pimpl)(EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS));
				}
				result_type operator ()(EPG_SIGNAL_SIGNATURE_FULL_ARGS(EPG_SIGNALS_NUM_ARGS, ~)) const
				{
					boost::shared_ptr<signalslib::detail::EPG_SIGNAL_IMPL_CLASS_NAME<EPG_SIGNAL_TEMPLATE_INSTANTIATION> >
						shared_pimpl(_weak_pimpl);
					return (*shared_pimpl)(EPG_SIGNAL_SIGNATURE_ARG_NAMES(EPG_SIGNALS_NUM_ARGS));
				}
			private:
				boost::weak_ptr<signalslib::detail::EPG_SIGNAL_IMPL_CLASS_NAME<EPG_SIGNAL_TEMPLATE_INSTANTIATION> >
					_weak_pimpl;
			};

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
#undef EPG_SIGNAL_IMPL_CLASS_NAME
#undef EPG_SIGNAL_SIGNATURE_ARG_NAME
#undef EPG_SIGNAL_SIGNATURE_FULL_ARG
#undef EPG_SIGNAL_SIGNATURE_FULL_ARGS
#undef EPG_SIGNAL_SIGNATURE_ARG_NAMES
#undef EPG_SIGNAL_TEMPLATE_DEFAULTED_DECL
#undef EPG_SIGNAL_TEMPLATE_DECL
#undef EPG_SIGNAL_TEMPLATE_INSTANTIATION
