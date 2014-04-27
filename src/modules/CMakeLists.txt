
file(GLOB moduleSrc *.cpp)

include_directories(. ..)

add_library(RfsModules ${moduleSrc})
target_link_libraries(RfsModules RfsLib)

