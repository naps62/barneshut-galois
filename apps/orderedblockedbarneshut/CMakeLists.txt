#
#app(orderedblockedbarneshut Barneshut.cpp EXTLIBS /workspace/pcosta/local/lib/libCGAL.so)
site_name(host)
MESSAGE(STATUS "${host}")
#string(COMPARE NOTEQUAL host 'naps62-mint' result)
#MESSAGE(STATUS "${result}")
if (NOT host STREQUAL "naps62-mint")
	MESSAGE(STATUS "asd")
	include_directories(/workspace/pcosta/local/include)
endif()

app(orderedblockedbarneshut Barneshut.cpp)