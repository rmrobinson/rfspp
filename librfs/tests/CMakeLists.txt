include_directories(. .. ../lib)

add_executable(DiscoveryTest DiscoveryTest.cpp)
target_link_libraries(DiscoveryTest rfs)

add_executable(LogTest LogTest.cpp)
target_link_libraries(LogTest rfs)

