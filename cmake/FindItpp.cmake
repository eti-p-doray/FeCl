if (NOT ITPP_FOUND)
  find_path(ITPP_INCLUDE_DIRS
    NAMES itpp/itbase.h
    PATHS
    /usr/pkgs64/include
    /usr/include
    /usr/local/include
    /opt/include
    /opt/local/include
  )
  find_library(ITPP_LIBRARIES
    NAMES itpp
    PATHS ${ITPP_DIR}/libs
    "${ITPP_DIR}\\win32\\lib"
    /usr/pkgs64/lib
    /usr/lib64
    /usr/lib
    /usr/local/lib
    /opt/lib
    /opt/local/lib
  )

  if (ITPP_LIBRARIES AND ITPP_INCLUDE_DIRS)
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
endif()