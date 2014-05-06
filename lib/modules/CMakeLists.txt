
list(APPEND MODULE_LIBRARIES "RfsLib")
list(APPEND MODULE_LIBRARIES "br")

file(GLOB MODULE_SRC "*.cpp")

find_package(WiringPi)

if(WIRINGPI_FOUND)
    include_directories(${WIRINGPI_INCLUDE_DIRS})
    list(APPEND MODULE_LIBRARIES "${WIRINGPI_LIBRARIES}")
else()
    message(WARNING "WiringPi not found, not building LiteBriteController")
    list(REMOVE_ITEM MODULE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/LiteBriteController.cpp")
endif()

add_library(RfsModules ${MODULE_SRC})
target_link_libraries(RfsModules ${MODULE_LIBRARIES})

