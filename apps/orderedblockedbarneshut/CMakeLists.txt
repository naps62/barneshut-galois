#
#app(orderedblockedbarneshut Barneshut.cpp EXTLIBS /workspace/pcosta/local/lib/libCGAL.so)
site_name(host)
string(COMPARE NOTEQUAL host 'naps62-mint' result)
if (NOT result)
	include_directories(/workspace/pcosta/local/include)
endif (NOT result)

app(orderedblockedbarneshut Barneshut.cpp)