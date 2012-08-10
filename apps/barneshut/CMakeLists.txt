site_name(host)
if (NOT host STREQUAL "naps62-mint")
include_directories(/workspace/pcosta/local/include /opt/apps/papi/4.1.2.1/include /home1/01885/pg19837/cgal_install/include)
	app(barneshut EXTLIBS /opt/apps/papi/4.1.2.1/lib/libpapi.a)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=\"c++0x\" -pedantic")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=\"c99\" -pedantic")
endif()

#	compiling flag for libCGAL
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frounding-math")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -frounding-math")