find_package(Boost COMPONENTS system thread)

file(GLOB libRfsSrc *.cpp)

include_directories(${BOOST_INCLUDE_DIRS})

add_library(rfs ${libRfsSrc})
target_link_libraries(rfs RfsProto ${Boost_SYSTEM_LIBRARY})

