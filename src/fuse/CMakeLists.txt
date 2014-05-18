
find_package(FUSE REQUIRED)

file(GLOB RfsFuseSrc "*.cpp")

include_directories(${FUSE_INCLUDE_DIR})
set(CMAKE_CXX_FLAGS "-D_FILE_OFFSET_BITS=64")

add_executable(RfsFuse ${RfsFuseSrc})
target_link_libraries(RfsFuse RfsLib ${FUSE_LIBRARIES})

