add_subdirectory(structs)

app(raytraceroldblocked main.cpp Config.cpp)

target_link_libraries(raytraceroldblocked STRUCTS26)
