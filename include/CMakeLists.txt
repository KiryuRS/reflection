find_package(Boost REQUIRED COMPONENTS HEADERS)

add_library(reflection_interface INTERFACE)
target_link_libraries(reflection_interface INTERFACE Boost::headers)
