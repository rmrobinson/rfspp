
include_directories(. ..)
add_executable(TimeOfDay TimeOfDay.cpp)
target_link_libraries(TimeOfDay RfsModules)

add_executable(LogTest LogTest.cpp)
target_link_libraries(LogTest utils)

