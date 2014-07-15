include_directories(. .. ../lib)

add_executable(testDiscovery TestDiscovery.cpp)
target_link_libraries(testDiscovery rfs utils)

