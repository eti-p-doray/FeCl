cmake_minimum_required (VERSION 2.8.11)

if(MATLAB_FOUND)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Matlab)
endif(MATLAB_FOUND)
