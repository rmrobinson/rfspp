include_directories(. ../lib ../include)

add_executable(DiscoveryTest DiscoveryTest.cpp)
target_link_libraries(DiscoveryTest rfs)

add_executable(LogTest LogTest.cpp)
target_link_libraries(LogTest rfs)

add_executable(SyncStatTest SyncStatTest.cpp)
target_link_libraries(SyncStatTest rfs)

