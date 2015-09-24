# Copyright (C) 2013 Bastian Bloessl <bloessl@ccs-labs.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


# ----------------------------------------------------------------------------
# FIND ITPP COMPONENTS
# ----------------------------------------------------------------------------
if (NOT ITPP_FOUND)
  find_path(ITPP_INCLUDE_DIR
    NAMES itpp/itbase.h
    HINTS ${ITPP_INCLUDEDIR}
    ${ITPP_INCLUDEDIR}/include
    ${ITPP_DIR}/include
    PATHS
    /usr/pkgs64/include
    /usr/include
    /usr/local/include
    /opt/include
    /opt/local/include
  )
  find_library(ITPP_LIBRARY_NORMAL
    NAMES itpp
    HINTS ${ITPP_LIBDIR}
    PATHS ${ITPP_DIR}/libs
    "${ITPP_DIR}\\win32\\lib"
    /usr/pkgs64/lib
    /usr/lib64
    /usr/lib
    /usr/local/lib
  )
  find_library(ITPP_LIBRARY_DEBUG
    NAMES itpp_debug
    HINTS ${ITPP_LIBDIR}
    PATHS ${ITPP_DIR}/libs
    "${ITPP_DIR}\\win32\\lib"
    /usr/pkgs64/lib
    /usr/lib64
    /usr/lib
    /usr/local/lib
  )

  if (ITPP_LIBRARY_NORMAL OR ITPP_LIBRARY_DEBUG)
    if ((CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo") AND ITPP_LIBRARY_DEBUG)
      SET (ITPP_LIBRARY ${ITPP_LIBRARY_DEBUG})
    else ((CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo") AND ITPP_LIBRARY_DEBUG)
      SET (ITPP_LIBRARY ${ITPP_LIBRARY_NORMAL})
    endif ((CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo") AND ITPP_LIBRARY_DEBUG)
  endif (ITPP_LIBRARY_NORMAL OR ITPP_LIBRARY_DEBUG)

  if (ITPP_LIBRARY AND ITPP_INCLUDE_DIR)
    SET(ITPP_LIBRARIES ${ITPP_LIBRARY})
    SET(ITPP_INCLUDE_DIRS ${ITPP_INCLUDE_DIR})
    SET(ITPP_FOUND "YES")
  else (ITPP_LIBRARY AND ITPP_INCLUDE_DIR)
    SET(ITPP_FOUND "NO")
  endif (ITPP_LIBRARY AND ITPP_INCLUDE_DIR)

  if (ITPP_FOUND)
    if (NOT ITPP_FIND_QUIETLY)
      MESSAGE(STATUS "Found ITPP: ${ITPP_LIBRARIES}")
    endif (NOT ITPP_FIND_QUIETLY)
  else (ITPP_FOUND)
    if (ITPP_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find ITPP library")
    endif (ITPP_FIND_REQUIRED)
  endif (ITPP_FOUND)

  mark_as_advanced(
    ITPP_LIBRARY
    ITPP_INCLUDE_DIR
  )
endif()