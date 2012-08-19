include_directories(/workspace/pcosta/local/include)
add_subdirectory(structs)

app(raytraceroldblocked main.cpp Config.cpp EXTLIBS /workspace/pcosta/local/lib/libpapi.a)

target_link_libraries(raytraceroldblocked STRUCTS26)
