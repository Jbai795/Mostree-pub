cmake_minimum_required (VERSION 3.18)

project(mostree VERSION 1.0.0)

set(CMAKE_VERBOSE_MAKEFILE ON)
message(STATUS "CMAKE_VERBOSE_MAKEFILE: ${CMAKE_VERBOSE_MAKEFILE}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_HOME_DIRECTORY: ${CMAKE_HOME_DIRECTORY}")

if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")

	############################################
	#          If top level cmake              #
	############################################

	set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
	set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
	set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

	if(MSVC)

	else()
		# set(CMAKE_C_FLAGS "-Wall  -maes -msse2 -msse4.1 -msse3 -mavx -mavx2 -mpclmul -Wfatal-errors -pthread -Wno-ignored-attributes -Wno-int-in-bool-context")
		set(CMAKE_C_FLAGS "-Wall  -maes -msse2 -msse4.1 -msse3 -mavx -mavx2 -mpclmul -Wfatal-errors -pthread -Wno-ignored-attributes ")
		set(CMAKE_CXX_FLAGS  "${CMAKE_C_FLAGS}  -std=c++14")

		# Select flags.
		SET(CMAKE_CXX_FLAGS_RELEASE "-Ofast -DNDEBUG") 
		SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO " -O2 -g -ggdb")
		SET(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g -ggdb")

	endif()

	# Set a default build type for single-configuration
	# CMake generators if no build type is set.
	if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	   SET(CMAKE_BUILD_TYPE Release)
	endif()

	if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
	  message(WARNING "Configuration NOT set to Release. Please call cmake with -DCMAKE_BUILD_TYPE=Release  instead of ${CMAKE_BUILD_TYPE}")

		if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		  #message(WARNING "Debug build.")
		elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release") 
		  #message(WARNING "Release build.")
		elseif("${CMAKE_BUILD_TYPE}" STREQUAL  "RELWITHDEBINFO")
		  #message(WARNING "RELWITHDEBINFO=Release with debug info build.")
		else()
		  message(ERROR ": Unknown build type - ${CMAKE_BUILD_TYPE}  Please use one of Debug, Release, or RELWITHDEBINFO\n\n" )
		endif()
	endif()
endif()

#############################################
#       Library/includes locations          #
#############################################

if(MSVC)
	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/thirdparty/win-debug/")
	endif()
	list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/thirdparty/win/")
else()
	list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/thirdparty/unix/")
endif()

set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/deps/build/")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
find_package(Boost 1.75.0   COMPONENTS system thread)

message(STATUS "Boost_FOUND " ${Boost_FOUND} ${Boost_Lib_Dirs})
message(STATUS "Boost_INCLUDE_DIR " ${Boost_INCLUDE_DIR})
message(STATUS "Boost_Lib_Dirs " ${Boost_Lib_Dirs})
message(STATUS "Boost_LIBRARIES " ${Boost_LIBRARIES})

message(STATUS "ENABLE_BOOST " ${ENABLE_BOOST})

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/deps/backward-cpp)
find_package(Backward)

find_package(libOTe HINTS "${libOTe_Dirs}")
if(NOT ENABLE_BOOST)
	message(FATAL_ERROR "libOTe/cryptoTools needs to have boost enabled.")
endif()

if(NOT ENABLE_CIRCUITS)
	message(FATAL_ERROR "libOTe/cryptoTools needs to have ENABLE_CIRCUITS defined.")
endif()

#############################################
#        Build aby3, mostree         		#
#############################################
#include_directories(${CMAKE_SOURCE_DIR}/deps/backward-cpp/)
add_subdirectory(aby3)
add_subdirectory(ss3-dtree)
add_subdirectory(dpf-offline)