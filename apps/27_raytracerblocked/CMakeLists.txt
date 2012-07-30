add_subdirectory(structs)

app(raytracerblocked main.cpp Config.cpp)

target_link_libraries(raytracerblocked STRUCTSBLOCKED)