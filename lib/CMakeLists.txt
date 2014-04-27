find_package(Boost REQUIRED)

file(GLOB libSrc *.cpp)

include_directories(${BOOST_INCLUDE_DIRS})

add_library(RfsLib ${libSrc})
target_link_libraries(RfsLib RfsProto ${BOOST_UUID_LIBRARY})

