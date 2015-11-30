cmake_minimum_required (VERSION 2.8.11)
#project(FeCl LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/) # add FindMatlab module
find_package(Matlab)
find_package(Doxygen)
find_package(Itpp)

set(Boost_USE_STATIC_LIBS       ON) # only find static libs
set(Boost_USE_STATIC_RUNTIME   	OFF)
add_definitions(-DBOOST_ALL_NO_LIB)

set(Boost_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/include/)
set(Boost_LIBRARIES serialization)

if (UNIX)
  if (APPLE)
    set(CMAKE_CXX_FLAGS "-mmacosx-version-min=10.7 ${CMAKE_CXX_FLAGS}")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-mmacosx-version-min=10.7 ${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}")
  else ()
    set(CMAKE_CXX_FLAGS "-pthread -fPIC ${CMAKE_CXX_FLAGS}")
  endif (APPLE)
  set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
endif (UNIX)

enable_testing()

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/serialization)
add_subdirectory(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_SOURCE_DIR}/test)
add_subdirectory(${CMAKE_SOURCE_DIR}/benchmark)
add_subdirectory(${CMAKE_SOURCE_DIR}/Wrap)
