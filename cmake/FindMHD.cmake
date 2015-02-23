# Find microhttpd
#
# Find the microhttpd includes and library
#
# if you need to add a custom library search path, do it via via CMAKE_PREFIX_PATH
#
# This module defines
#  MHD_INCLUDE_DIRS, where to find header, etc.
#  MHD_LIBRARIES, the libraries needed to use jsoncpp.
#  MHD_FOUND, If false, do not try to use jsoncpp.

find_path(
	MHD_INCLUDE_DIR
	NAMES microhttpd.h
	DOC "microhttpd include dir"
)

find_library(
	MHD_LIBRARY
	NAMES microhttpd microhttpd-10 libmicrohttpd libmicrohttpd-dll
	DOC "microhttpd library"
)

set(MHD_INCLUDE_DIRS ${MHD_INCLUDE_DIR})
set(MHD_LIBRARIES ${MHD_LIBRARY})

# debug library on windows
# same naming convention as in QT (appending debug library with d)
# boost is using the same "hack" as us with "optimized" and "debug"
# official MHD project actually uses _d suffix
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

	#TODO: place dlls into CMAKE_CFG_INTDIR subfolders
	string(REPLACE ".lib" ".dll" MHD_DLL_RELEASE ${MHD_LIBRARY})
	string(REPLACE "/lib/" "/bin/" MHD_DLL_RELEASE ${MHD_DLL_RELEASE})

	find_library(
		MHD_LIBRARY_DEBUG
		NAMES microhttpd_d microhttpd-10_d libmicrohttpd_d libmicrohttpd-dll_d
		DOC "mhd debug library"
	)
	# always use release for now
	#string(REPLACE ".lib" ".dll" MHD_DLL_DEBUG ${MHD_LIBRARY_DEBUG})
	#set(MHD_LIBRARIES optimized ${MHD_LIBRARIES} debug ${MHD_LIBRARY_DEBUG})

endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mhd DEFAULT_MSG
	MHD_INCLUDE_DIR MHD_LIBRARY)

mark_as_advanced(MHD_INCLUDE_DIR MHD_LIBRARY)

