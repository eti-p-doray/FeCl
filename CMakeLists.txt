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
  set(CMAKE_CXX_FLAGS " -std=c++11 -pthread -fPIC ${CMAKE_CXX_FLAGS}")
endif (UNIX)

enable_testing()

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/serialization)
add_subdirectory(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_SOURCE_DIR}/test)
add_subdirectory(${CMAKE_SOURCE_DIR}/benchmark)
