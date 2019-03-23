# Copyright 2019 Mike Dev
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt
#
# NOTE: CMake support for Boost.Signals2 is currently experimental at best
#       and the interface is likely to change in the future

file( GLOB test_src_files *.cpp )

list(REMOVE_ITEM test_src_files ${CMAKE_CURRENT_LIST_DIR}/invocation_benchmark.cpp)

find_package(Threads REQUIRED)

foreach( file IN LISTS test_src_files )

	get_filename_component( test_name ${file} NAME_WE )
	set( executable_name test_boost_signals2_${test_name} )
	add_executable( ${executable_name} ${file} )
	target_link_libraries( ${executable_name}
		PUBLIC
			Boost::signals2
			Boost::test
			Boost::timer
			Threads::Threads
	)
	add_test( NAME ${executable_name} COMMAND ${executable_name} )

endforeach()
