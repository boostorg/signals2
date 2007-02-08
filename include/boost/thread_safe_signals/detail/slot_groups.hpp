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
#include <map>
#include <boost/shared_ptr.hpp>
#include <utility>

namespace EPG {
	namespace signalslib {
		namespace detail {
			enum slot_meta_group {front_ungrouped_slots, grouped_slots, back_ungrouped_slots};
			template<typename Group, typename GroupCompare>
			class slot_group_map
			{
			public:
				typedef std::pair<enum slot_meta_group, Group> key_type;
				struct group_key_less
				{
					bool operator ()(const key_type &key1, const key_type &key2) const
					{
						if(key1.first < key2.first) return true;
						return key1.second < key2.second;
					}
				};
				typedef std::multimap<key_type, boost::shared_ptr<ConnectionBodyBase>,
					group_key_less > map_type;
			};
		} // end namespace detail
		enum connect_position { at_back, at_front };
	} // end namespace signalslib
} // end namespace EPG

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_TSS_SLOT_GROUPS_HEADER
