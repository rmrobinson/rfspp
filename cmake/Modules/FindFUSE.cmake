# - Try to find the FUSE file system in userspace library
# Once done this will define
#
#  FUSE_FOUND - System has FUSE
#  FUSE_INCLUDE_DIR - The FUSE include directory
#  FUSE_LIBRARIES - The libraries needed to use FUSE

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Alexander Neundorf <neundorf@kde.org>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
FIND_PACKAGE(PkgConfig QUIET)
PKG_CHECK_MODULES(PC_FUSE QUIET fuse)
SET(FUSE_DEFINITIONS ${PC_FUSE_CFLAGS_OTHER})

FIND_PATH(FUSE_INCLUDE_DIR NAMES fuse.h
   HINTS
   ${PC_FUSE_INCLUDEDIR}
   ${PC_FUSE_INCLUDE_DIRS}
   PATH_SUFFIXES fuse
   )

FIND_LIBRARY(FUSE_LIBRARIES NAMES fuse libosxfuse.dylib
   HINTS
   ${PC_FUSE_LIBDIR}
   ${PC_FUSE_LIBRARY_DIRS}
   )

# handle the QUIETLY and REQUIRED arguments and set FUSE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FUSE REQUIRED_VARS FUSE_LIBRARIES FUSE_INCLUDE_DIR) 

MARK_AS_ADVANCED(FUSE_INCLUDE_DIR FUSE_LIBRARIES)

