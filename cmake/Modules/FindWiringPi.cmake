# - Find wiringPi
# Find the native wiringPi header and library.
#
# WIRINGPI_INCLUDE_DIRS     - where to find wringPi.h, softPwm.h
# WIRINGPI_LIBRARIES        - libraries to link against.
# WIRINGPI_FOUND            - true if wiringPi found.
#

# This is based on the FindCURL.cmake file distributed with Cmake.

find_path(WIRINGPI_INCLUDE_DIR NAMES wiringPi.h softPwm.h)
find_library(WIRINGPI_LIBRARY NAMES wiringPi)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(WIRINGPI
                                  REQUIRED_VARS WIRINGPI_INCLUDE_DIR WIRINGPI_LIBRARY)

if(WIRINGPI_FOUND)
    set(WIRINGPI_INCLUDE_DIRS ${WIRINGPI_INCLUDE_DIR})
    set(WIRINGPI_LIBRARIES ${WIRINGPI_LIBRARY} pthread)
endif()

