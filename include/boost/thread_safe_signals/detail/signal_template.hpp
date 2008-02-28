/*
  Template for Signa1, Signal2, ... classes that support signals
  with 1, 2, ... parameters

  Author: Frank Mori Hess <fmhess@users.sourceforge.net>
  Begin: 2007-01-23
*/
// Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is included iteratively, and should not be protected from multiple inclusion

#define BOOST_SIGNALS_NUM_ARGS BOOST_PP_ITERATION()

#define BOOST_SIGNAL_CLASS_NAME BOOST_PP_CAT(signal, BOOST_SIGNALS_NUM_ARGS)
#define BOOST_WEAK_SIGNAL_CLASS_NAME BOOST_PP_CAT(weak_, BOOST_SIGNAL_CLASS_NAME)
#define BOOST_SIGNAL_IMPL_CLASS_NAME BOOST_PP_CAT(BOOST_SIGNAL_CLASS_NAME, _impl)

// typename R, typename T1, typename T2, ..., typename TN, typename Combiner = boost::last_value<R>, ...
#define BOOST_SIGNAL_TEMPLATE_DEFAULTED_DECL \
  BOOST_SIGNAL_SIGNATURE_TEMPLATE_DECL(BOOST_SIGNALS_NUM_ARGS), \
  typename Combiner = last_value<R>, \
  typename Group = int, \
  typename GroupCompare = std::less<Group>, \
  typename SlotFunction = BOOST_FUNCTION_N_DECL(BOOST_SIGNALS_NUM_ARGS), \
  typename ThreadingModel = signalslib::auto_threaded
// typename R, typename T1, typename T2, ..., typename TN, typename Combiner, ...
#define BOOST_SIGNAL_TEMPLATE_DECL \
  BOOST_SIGNAL_SIGNATURE_TEMPLATE_DECL(BOOST_SIGNALS_NUM_ARGS), \
  typename Combiner, \
  typename Group, \
  typename GroupCompare, \
  typename SlotFunction, \
  typename ThreadingModel
// R, T1, T2, ..., TN, Combiner, Group, GroupCompare, SlotFunction, ThreadingModel
#define BOOST_SIGNAL_TEMPLATE_INSTANTIATION \
  BOOST_SIGNAL_SIGNATURE_TEMPLATE_INSTANTIATION(BOOST_SIGNALS_NUM_ARGS), \
  Combiner, Group, GroupCompare, SlotFunction, ThreadingModel

namespace boost
{
  namespace signalslib
  {
    namespace detail
    {
      template<BOOST_SIGNAL_TEMPLATE_DECL>
      class BOOST_SIGNAL_IMPL_CLASS_NAME
      {
      public:
        typedef SlotFunction slot_function_type;
        // typedef slotN<Signature, SlotFunction> slot_type;
        typedef BOOST_SLOT_CLASS_NAME(BOOST_SIGNALS_NUM_ARGS)<BOOST_SIGNAL_SIGNATURE_TEMPLATE_INSTANTIATION(BOOST_SIGNALS_NUM_ARGS),
          slot_function_type> slot_type;
      private:
        class slot_invoker;
        typedef typename signalslib::detail::group_key<Group>::type group_key_type;
        typedef shared_ptr<ConnectionBody<group_key_type, slot_type, ThreadingModel> > connection_body_type;
        typedef grouped_list<Group, GroupCompare, connection_body_type> connection_list_type;
      public:
        typedef typename slot_function_type::result_type slot_result_type;
        typedef Combiner combiner_type;
        typedef typename combiner_type::result_type result_type;
        typedef Group group_type;
        typedef GroupCompare group_compare_type;
        typedef typename signalslib::detail::slot_call_iterator_t<slot_invoker,
          typename connection_list_type::iterator, ConnectionBody<group_key_type, slot_type, ThreadingModel> > slot_call_iterator;

        BOOST_SIGNAL_IMPL_CLASS_NAME(const combiner_type &combiner,
          const group_compare_type &group_compare):
          _shared_state(new invocation_state(connection_list_type(group_compare), combiner)),
          _garbage_collector_it(_shared_state->connection_bodies.end())
        {}
        // connect slot
        signalslib::connection connect(const slot_type &slot, signalslib::connect_position position = signalslib::at_back)
        {
          typename mutex_type::scoped_lock lock(_mutex);
          connection_body_type newConnectionBody =
            create_new_connection(slot);
          group_key_type group_key;
          if(position == signalslib::at_back)
          {
            group_key.first = signalslib::detail::back_ungrouped_slots;
            _shared_state->connection_bodies.push_back(group_key, newConnectionBody);
          }else
          {
            group_key.first = signalslib::detail::front_ungrouped_slots;
            _shared_state->connection_bodies.push_front(group_key, newConnectionBody);
          }
          newConnectionBody->set_group_key(group_key);
          return signalslib::connection(newConnectionBody);
        }
        signalslib::connection connect(const group_type &group,
          const slot_type &slot, signalslib::connect_position position = signalslib::at_back)
        {
          typename mutex_type::scoped_lock lock(_mutex);
          connection_body_type newConnectionBody =
            create_new_connection(slot);
          // update map to first connection body in group if needed
          group_key_type group_key(signalslib::detail::grouped_slots, group);
          newConnectionBody->set_group_key(group_key);
          if(position == signalslib::at_back)
          {
            _shared_state->connection_bodies.push_back(group_key, newConnectionBody);
          }else  // at_front
          {
            _shared_state->connection_bodies.push_front(group_key, newConnectionBody);
          }
          return signalslib::connection(newConnectionBody);
        }
        // disconnect slot(s)
        void disconnect_all_slots()
        {
          shared_ptr<invocation_state> local_state =
            get_readable_state();
          typename connection_list_type::iterator it;
          for(it = local_state->connection_bodies.begin();
            it != local_state->connection_bodies.end(); ++it)
          {
            (*it)->disconnect();
          }
        }
        void disconnect(const group_type &group)
        {
          shared_ptr<invocation_state> local_state =
            get_readable_state();
          group_key_type group_key(signalslib::detail::grouped_slots, group);
          typename connection_list_type::iterator it;
          typename connection_list_type::iterator end_it =
            local_state->connection_bodies.upper_bound(group_key);
          for(it = local_state->connection_bodies.lower_bound(group_key);
            it != end_it; ++it)
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
        result_type operator ()(BOOST_SIGNAL_SIGNATURE_FULL_ARGS(BOOST_SIGNALS_NUM_ARGS))
        {
          shared_ptr<invocation_state> local_state;
          typename connection_list_type::iterator it;
          {
            typename mutex_type::scoped_lock listLock(_mutex);
            // only clean up if it is safe to do so
            if(_shared_state.unique())
              nolock_cleanup_connections(false);
            /* Make a local copy of _shared_state while holding mutex, so we are
            thread safe against the combiner or connection list getting modified
            during invocation. */
            local_state = _shared_state;
          }
          slot_invoker invoker BOOST_PP_IF(BOOST_SIGNALS_NUM_ARGS, \
            (BOOST_SIGNAL_SIGNATURE_ARG_NAMES(BOOST_SIGNALS_NUM_ARGS)), );
          optional<typename signalslib::detail::slot_result_type_wrapper<slot_result_type>::type > cache;
          return local_state->combiner(
            slot_call_iterator(local_state->connection_bodies.begin(), local_state->connection_bodies.end(), invoker, cache),
            slot_call_iterator(local_state->connection_bodies.end(), local_state->connection_bodies.end(), invoker, cache));
        }
        result_type operator ()(BOOST_SIGNAL_SIGNATURE_FULL_ARGS(BOOST_SIGNALS_NUM_ARGS)) const
        {
          shared_ptr<invocation_state> local_state;
          typename connection_list_type::iterator it;
          {
            typename mutex_type::scoped_lock listLock(_mutex);
            // only clean up if it is safe to do so
            if(_shared_state.unique())
              nolock_cleanup_connections(false);
            /* Make a local copy of _shared_state while holding mutex, so we are
            thread safe against the combiner or connection list getting modified
            during invocation. */
            local_state = _shared_state;
          }
          slot_invoker invoker BOOST_PP_IF(BOOST_SIGNALS_NUM_ARGS, \
            (BOOST_SIGNAL_SIGNATURE_ARG_NAMES(BOOST_SIGNALS_NUM_ARGS)), );
          optional<typename signalslib::detail::slot_result_type_wrapper<slot_result_type>::type > cache;
          return const_cast<const combiner_type&>(local_state->combiner)(
            slot_call_iterator(local_state->connection_bodies.begin(), local_state->connection_bodies.end(), invoker, cache),
            slot_call_iterator(local_state->connection_bodies.end(), local_state->connection_bodies.end(), invoker, cache));
        }
        std::size_t num_slots() const
        {
          shared_ptr<invocation_state> local_state =
            get_readable_state();
          typename connection_list_type::iterator it;
          std::size_t count = 0;
          for(it = local_state->connection_bodies.begin();
            it != local_state->connection_bodies.end(); ++it)
          {
            if((*it)->connected()) ++count;
          }
          return count;
        }
        bool empty() const
        {
          shared_ptr<invocation_state> local_state =
            get_readable_state();
          typename connection_list_type::iterator it;
          for(it = local_state->connection_bodies.begin();
            it != local_state->connection_bodies.end(); ++it)
          {
            if((*it)->connected()) return false;
          }
          return true;
        }
        combiner_type combiner() const
        {
          typename mutex_type::scoped_lock lock(_mutex);
          return _shared_state->combiner;
        }
        void set_combiner(const combiner_type &combiner)
        {
          typename mutex_type::scoped_lock lock(_mutex);
          if(_shared_state.unique())
            _shared_state->combiner = combiner;
          else
            _shared_state.reset(new invocation_state(_shared_state->connection_bodies, combiner));
        }
      private:
        typedef typename ThreadingModel::mutex_type mutex_type;

        // slot_invoker is passed to slot_call_iterator_t to run slots
        class slot_invoker
        {
        public:
          typedef typename signalslib::detail::slot_result_type_wrapper<slot_result_type>::type result_type;

          slot_invoker(BOOST_SIGNAL_SIGNATURE_FULL_ARGS(BOOST_SIGNALS_NUM_ARGS)) BOOST_PP_IF(BOOST_SIGNALS_NUM_ARGS, :, )
// argn ( argn ) ,
#define BOOST_SIGNAL_MISC_STATEMENT(z, n, data) \
  BOOST_PP_CAT(arg, n) ( BOOST_PP_CAT(arg, n) )
// arg1(arg1), arg2(arg2), ..., argn(argn)
            BOOST_PP_ENUM_SHIFTED(BOOST_PP_INC(BOOST_SIGNALS_NUM_ARGS), BOOST_SIGNAL_MISC_STATEMENT, ~)
#undef BOOST_SIGNAL_MISC_STATEMENT
          {}
          result_type operator ()(const connection_body_type &connectionBody) const
          {
            result_type *resolver = 0;
            return m_invoke(connectionBody,
              resolver);
          }
// Tn argn;
#define BOOST_SIGNAL_MISC_STATEMENT(z, n, Signature) \
  BOOST_PP_CAT(T, BOOST_PP_INC(n)) BOOST_SIGNAL_SIGNATURE_ARG_NAME(~, n, ~);
          BOOST_PP_REPEAT(BOOST_SIGNALS_NUM_ARGS, BOOST_SIGNAL_MISC_STATEMENT, ~)
#undef BOOST_SIGNAL_MISC_STATEMENT
        private:
          result_type m_invoke(const connection_body_type &connectionBody,
            const signalslib::detail::unusable *) const
          {
            connectionBody->slot.slot_function()(BOOST_SIGNAL_SIGNATURE_ARG_NAMES(BOOST_SIGNALS_NUM_ARGS));
            return signalslib::detail::unusable();
          }
          result_type m_invoke(const connection_body_type &connectionBody, ...) const
          {
            return connectionBody->slot.slot_function()(BOOST_SIGNAL_SIGNATURE_ARG_NAMES(BOOST_SIGNALS_NUM_ARGS));
          }
        };
        // a struct used to optimize (minimize) the number of shared_ptrs that need to be created
        // inside operator()
        struct invocation_state
        {
          invocation_state(const connection_list_type &connections,
            const combiner_type &combiner): connection_bodies(connections),
            combiner(combiner)
          {}
          invocation_state(const invocation_state &other):
            connection_bodies(other.connection_bodies),
            combiner(other.combiner)
          {}
          connection_list_type connection_bodies;
          combiner_type combiner;
        };

        // clean up disconnected connections
        void nolock_cleanup_connections(bool grab_tracked,
          const typename connection_list_type::iterator &begin, bool break_on_connected = false) const
        {
          BOOST_ASSERT(_shared_state.unique());
          typename connection_list_type::iterator it;
          for(it = begin; it != _shared_state->connection_bodies.end();)
          {
            bool connected;
            {
              typename ConnectionBody<group_key_type, slot_type, ThreadingModel>::mutex_type::scoped_lock lock((*it)->mutex);
              if(grab_tracked)
                (*it)->nolock_slot_expired();
              connected = (*it)->nolock_nograb_connected();
            }// scoped lock destructs here, safe to erase now
            if(connected == false)
            {
              it = _shared_state->connection_bodies.erase((*it)->group_key(), it);
            }else
            {
              ++it;
              if(break_on_connected) break;
            }
          }
          _garbage_collector_it = it;
        }
        // clean up a few connections in constant time
        void nolock_cleanup_connections(bool grab_tracked) const
        {
          BOOST_ASSERT(_shared_state.unique());
          typename connection_list_type::iterator begin;
          if(_garbage_collector_it == _shared_state->connection_bodies.end())
          {
            begin = _shared_state->connection_bodies.begin();
          }else
          {
            begin = _garbage_collector_it;
          }
          nolock_cleanup_connections(grab_tracked, begin, true);
        }
        /* Make a new copy of the slot list if it is currently being read somewhere else
        */
        void nolock_force_unique_connection_list()
        {
          if(_shared_state.unique() == false)
          {
            _shared_state = shared_ptr<invocation_state>(new invocation_state(*_shared_state));
            nolock_cleanup_connections(true, _shared_state->connection_bodies.begin());
          }else
          {
            nolock_cleanup_connections(true);
          }
        }
        shared_ptr<invocation_state> get_readable_state() const
        {
          typename mutex_type::scoped_lock listLock(_mutex);
          return _shared_state;
        }
        connection_body_type create_new_connection(const slot_type &slot)
        {
          nolock_force_unique_connection_list();
          return connection_body_type(new ConnectionBody<group_key_type, slot_type, ThreadingModel>(slot));
        }
        void do_disconnect(const group_type &group, mpl::bool_<true> is_group)
        {
          disconnect(group);
        }
        template<typename T>
        void do_disconnect(const T &slot, mpl::bool_<false> is_group)
        {
          shared_ptr<invocation_state> local_state =
            get_readable_state();
          typename connection_list_type::iterator it;
          for(it = local_state->connection_bodies.begin();
            it != local_state->connection_bodies.end(); ++it)
          {
            typename ConnectionBody<group_key_type, slot_type, ThreadingModel>::mutex_type::scoped_lock lock((*it)->mutex);
            if((*it)->slot.slot_function() == slot)
            {
              (*it)->nolock_disconnect();
            }
          }
        }

        shared_ptr<invocation_state> _shared_state;
        mutable typename connection_list_type::iterator _garbage_collector_it;
        // connection list mutex must never be locked when attempting a blocking lock on a slot,
        // or you could deadlock.
        mutable mutex_type _mutex;
      };

      template<BOOST_SIGNAL_TEMPLATE_DECL>
      class BOOST_WEAK_SIGNAL_CLASS_NAME;
    }
  }

  template<BOOST_SIGNAL_TEMPLATE_DEFAULTED_DECL>
  class BOOST_SIGNAL_CLASS_NAME: public signalslib::signal_base
  {
  public:
    typedef signalslib::detail::BOOST_WEAK_SIGNAL_CLASS_NAME<BOOST_SIGNAL_TEMPLATE_INSTANTIATION> weak_signal_type;
    friend class signalslib::detail::BOOST_WEAK_SIGNAL_CLASS_NAME<BOOST_SIGNAL_TEMPLATE_INSTANTIATION>;

    typedef SlotFunction slot_function_type;
    // typedef slotN<Signature, SlotFunction> slot_type;
    typedef BOOST_SLOT_CLASS_NAME(BOOST_SIGNALS_NUM_ARGS)<BOOST_SIGNAL_SIGNATURE_TEMPLATE_INSTANTIATION(BOOST_SIGNALS_NUM_ARGS),
      slot_function_type> slot_type;
    typedef typename slot_function_type::result_type slot_result_type;
    typedef Combiner combiner_type;
    typedef typename combiner_type::result_type result_type;
    typedef Group group_type;
    typedef GroupCompare group_compare_type;
    typedef typename signalslib::detail::BOOST_SIGNAL_IMPL_CLASS_NAME<BOOST_SIGNAL_TEMPLATE_INSTANTIATION>::slot_call_iterator
      slot_call_iterator;
// typedef Tn argn_type;
#define BOOST_SIGNAL_MISC_STATEMENT(z, n, data) \
  typedef BOOST_PP_CAT(T, BOOST_PP_INC(n)) BOOST_PP_CAT(BOOST_PP_CAT(arg, BOOST_PP_INC(n)), _type);
        BOOST_PP_REPEAT(BOOST_SIGNALS_NUM_ARGS, BOOST_SIGNAL_MISC_STATEMENT, ~)
#undef BOOST_SIGNAL_MISC_STATEMENT
#if BOOST_SIGNALS_NUM_ARGS == 1
    typedef arg1_type argument_type;
#elif BOOST_SIGNALS_NUM_ARGS == 2
    typedef arg1_type first_argument_type;
    typedef arg2_type second_argument_type;
#endif
    BOOST_STATIC_CONSTANT(int, arity = BOOST_SIGNALS_NUM_ARGS);

    BOOST_SIGNAL_CLASS_NAME(const combiner_type &combiner = combiner_type(),
      const group_compare_type &group_compare = group_compare_type()):
      _pimpl(new signalslib::detail::BOOST_SIGNAL_IMPL_CLASS_NAME<BOOST_SIGNAL_TEMPLATE_INSTANTIATION>(combiner, group_compare))
    {};
    virtual ~BOOST_SIGNAL_CLASS_NAME()
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
    result_type operator ()(BOOST_SIGNAL_SIGNATURE_FULL_ARGS(BOOST_SIGNALS_NUM_ARGS))
    {
      return (*_pimpl)(BOOST_SIGNAL_SIGNATURE_ARG_NAMES(BOOST_SIGNALS_NUM_ARGS));
    }
    result_type operator ()(BOOST_SIGNAL_SIGNATURE_FULL_ARGS(BOOST_SIGNALS_NUM_ARGS)) const
    {
      return (*_pimpl)(BOOST_SIGNAL_SIGNATURE_ARG_NAMES(BOOST_SIGNALS_NUM_ARGS));
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
  protected:
    virtual shared_ptr<void> lock_pimpl() const
    {
      return _pimpl;
    }
  private:
    shared_ptr<signalslib::detail::BOOST_SIGNAL_IMPL_CLASS_NAME<BOOST_SIGNAL_TEMPLATE_INSTANTIATION> >
      _pimpl;
  };

  namespace signalslib
  {
    namespace detail
    {
      // wrapper class for storing other signals as slots with automatic lifetime tracking
      template<BOOST_SIGNAL_TEMPLATE_DECL>
      class BOOST_WEAK_SIGNAL_CLASS_NAME
      {
      public:
        typedef SlotFunction slot_function_type;
        typedef typename slot_function_type::result_type slot_result_type;
        typedef typename BOOST_SIGNAL_CLASS_NAME<BOOST_SIGNAL_TEMPLATE_INSTANTIATION>::result_type
          result_type;

        BOOST_WEAK_SIGNAL_CLASS_NAME(const BOOST_SIGNAL_CLASS_NAME<BOOST_SIGNAL_TEMPLATE_INSTANTIATION>
          &signal):
          _weak_pimpl(signal._pimpl)
        {}
        result_type operator ()(BOOST_SIGNAL_SIGNATURE_FULL_ARGS(BOOST_SIGNALS_NUM_ARGS))
        {
          shared_ptr<signalslib::detail::BOOST_SIGNAL_IMPL_CLASS_NAME<BOOST_SIGNAL_TEMPLATE_INSTANTIATION> >
            shared_pimpl(_weak_pimpl.lock());
          if(shared_pimpl == 0) throw expired_slot();
          return (*shared_pimpl)(BOOST_SIGNAL_SIGNATURE_ARG_NAMES(BOOST_SIGNALS_NUM_ARGS));
        }
        result_type operator ()(BOOST_SIGNAL_SIGNATURE_FULL_ARGS(BOOST_SIGNALS_NUM_ARGS)) const
        {
          shared_ptr<signalslib::detail::BOOST_SIGNAL_IMPL_CLASS_NAME<BOOST_SIGNAL_TEMPLATE_INSTANTIATION> >
            shared_pimpl(_weak_pimpl.lock());
          if(shared_pimpl == 0) throw expired_slot();
          return (*shared_pimpl)(BOOST_SIGNAL_SIGNATURE_ARG_NAMES(BOOST_SIGNALS_NUM_ARGS));
        }
      private:
        boost::weak_ptr<signalslib::detail::BOOST_SIGNAL_IMPL_CLASS_NAME<BOOST_SIGNAL_TEMPLATE_INSTANTIATION> >
          _weak_pimpl;
      };

      template<unsigned arity, typename Signature, typename Combiner,
        typename Group, typename GroupCompare, typename SlotFunction, typename ThreadingModel>
      class signalN;
      // partial template specialization
      template<typename Signature, typename Combiner, typename Group,
        typename GroupCompare, typename SlotFunction, typename ThreadingModel>
      class signalN<BOOST_SIGNALS_NUM_ARGS, Signature, Combiner, Group,
        GroupCompare, SlotFunction, ThreadingModel>
      {
      public:
        typedef BOOST_SIGNAL_CLASS_NAME<
          BOOST_SIGNAL_PORTABLE_SIGNATURE(BOOST_SIGNALS_NUM_ARGS, Signature),
          Combiner, Group,
          GroupCompare, SlotFunction, ThreadingModel> type;
      };
    }
  }
}

#undef BOOST_SIGNALS_NUM_ARGS
#undef BOOST_SIGNAL_CLASS_NAME
#undef BOOST_WEAK_SIGNAL_CLASS_NAME
#undef BOOST_SIGNAL_IMPL_CLASS_NAME
#undef BOOST_SIGNAL_TEMPLATE_DEFAULTED_DECL
#undef BOOST_SIGNAL_TEMPLATE_DECL
#undef BOOST_SIGNAL_TEMPLATE_INSTANTIATION
