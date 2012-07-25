add_subdirectory(structs)

app(raytracerstructured main.cpp Config.cpp)

target_link_libraries(raytracerstructured STRUCTS)