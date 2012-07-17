site_name(host)
if (NOT host STREQUAL "naps62-mint")
	include_directories(/workspace/pcosta/local/include)
endif()

app(orderedblockedbarneshut Barneshut.cpp)