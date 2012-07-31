add_subdirectory(structs)

app(raytracerblocked main.cpp Config.cpp EXTLIBS /usr/local/lib/libCGAL.so)

target_link_libraries(raytracerblocked STRUCTSBLOCKED)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frounding-math")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -frounding-math")
endif()