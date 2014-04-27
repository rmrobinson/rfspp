
find_package(Protobuf REQUIRED)

file(GLOB LibRfsProto "*.proto")

PROTOBUF_GENERATE_CPP(LibRfsProtoSrc LibRfsProtoHdr ${LibRfsProto})

set(RfsProto_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

add_library(RfsProto ${LibRfsProtoSrc})
target_link_libraries(RfsProto ${PROTOBUF_LIBRARY})

