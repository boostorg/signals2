// Thread-safe signals library

// Copyright Frank Mori Hess 2007. Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#ifndef BOOST_TSS_SLOT_GROUPS_HEADER
#define BOOST_TSS_SLOT_GROUPS_HEADER

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

#include <boost/thread_safe_signals/connection.hpp>
#include <boost/optional.hpp>
#include <utility>

namespace boost {
	namespace signalslib {
		namespace detail {
			enum slot_meta_group {front_ungrouped_slots, grouped_slots, back_ungrouped_slots};
			template<typename Group>
			struct group_key
			{
				typedef std::pair<enum slot_meta_group, boost::optional<Group> > type;
			};
			template<typename Group, typename GroupCompare>
			class group_key_less
			{
			public:
				group_key_less()
				{}
				group_key_less(const GroupCompare &group_compare): _group_compare(group_compare)
				{}
				bool operator ()(const typename group_key<Group>::type &key1, const typename group_key<Group>::type &key2) const
				{
					if(key1.first < key2.first) return true;
					if(key1.first != grouped_slots) return false;
					return _group_compare(key1.second.get(), key2.second.get());
				}
			private:
				GroupCompare _group_compare;
			};
		} // end namespace detail
		enum connect_position { at_back, at_front };
	} // end namespace signalslib
} // end namespace EPG

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TSS_SLOT_GROUPS_HEADER
