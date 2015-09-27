# CMake build control file for Serialization Library tests

cmake_minimum_required(VERSION 2.8.11)


#
# Compiler settings
#

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(UNIX)

if (UNIX)
  set(CMAKE_CXX_FLAGS " -std=c++11 -pthread -fPIC ${CMAKE_CXX_FLAGS}")
  add_definitions( -ftemplate-depth=300 )
else()
  add_definitions( /wd4996 )
endif (UNIX)

#
# IDE settings
#

if( CMAKE_HOST_APPLE )
  # note: it seems that bjam builds both address models in any case
  # so we can defer this decision to the IDE just as we do for debug/release
  # so we'll not use this now
  # set(Boost_ADDRESS_MODEL 64 CACHE INTEGER "32/64 bits")
  set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Link to Boost static libraries")
  set(Boost_USE_MULTITHREADED ON)
else()
  set(Boost_ADDRESS_MODEL 64 CACHE INTEGER "32/64 bits")
  set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Link to Boost static libraries")
  set(Boost_USE_MULTITHREADED ON)
endif()

#
# Locate Project Prerequisites 
#

# Boost

# note: we're assuming that boost has been built with:
# ./b2 —-layout=versioned toolset=clang-darwin link=static,shared variant=debug,release stage

###########################
# special notes for Xcode.

# these three should result in CMake setting the variables
# Boost_SERIALIZATION_LIBRARY_DEBUG … to the correct values.  

# But my current version of CMake doesn't automatically set the library names
# to point to the the libraries to be used.  The variables are created
# but they are not initialized.  So be prepared to set these variables by hand.
# If you want to use the static libraries - point to the boost libraries ending
# in ".a".  If you want to use the shared boost libraries - point to the libraries
# ending in ".dylib".

# But wait - there's more.
# if both lib.a and lib.dylib both exist in the library directory, Xcode will
# automatically chose the *.dylib one - and there's nothing you can do to fix this.
# So my recommendation is 
# a) to place the compiled libraries in two different directories
#    - e.g. stage/lib-static/*.a and stage/lib-shared/*.dylib
#    and set the CMake variable Boost_LIBRARY_DIR to point to one or the other
# b) create two different CMake build directories - build-static and build-shared
#    and switch between projects as desired.  I like to test both since
#    there are things like dead code elimination and visibility which vary
#    between the two environments.
#
#    caveat - as I write this, I've been unable to get the tests on the shared
#    library to function. Problem is that one needs to either put the shared
#    libraries in a special known place or set an environmental
#    variable which points to the shared library directory.  I prefer the latter
#    but I've been unable to figure out how to get Xcode to do on a global basis
#    and it's not practical to do this for 247 test targets one at a time.

# c) The targets in the project will by default be built as universal 32/64 binaries
#    I have yet to experiment with these yet so I just set the target to 64 bit.

# end special note for Xcode
############################

set(Boost_INCLUDE_DIRS	${CMAKE_SOURCE_DIR}/libs/include)

if(Boost_USE_STATIC_LIBS)
  set(BUILD_SHARED_LIBRARIES OFF)
else()
  set(BUILD_SHARED_LIBRARIES ON)
endif()

###########################
# library builds

add_library(serialization
  src/archive_exception.cpp
  src/basic_archive.cpp
  src/basic_iarchive.cpp
  src/basic_iserializer.cpp
  src/basic_oarchive.cpp
  src/basic_oserializer.cpp
  src/basic_pointer_iserializer.cpp
  src/basic_pointer_oserializer.cpp
  src/basic_serializer_map.cpp
  src/basic_text_iprimitive.cpp
  src/basic_text_oprimitive.cpp
  src/basic_xml_archive.cpp
  src/binary_iarchive.cpp
  src/binary_oarchive.cpp
  src/codecvt_null.cpp
  src/extended_type_info_no_rtti.cpp
  src/extended_type_info_typeid.cpp
  src/extended_type_info.cpp
  src/polymorphic_iarchive.cpp
  src/polymorphic_oarchive.cpp
  src/stl_port.cpp
  src/text_iarchive.cpp
  src/text_oarchive.cpp
  src/text_wiarchive.cpp
  src/text_woarchive.cpp
  src/utf8_codecvt_facet.cpp
  src/void_cast.cpp
  src/xml_archive_exception.cpp
  src/xml_iarchive.cpp
  src/xml_oarchive.cpp
  src/xml_grammar.cpp
  # src/basic_xml_grammar.ipp # doesn't show up in "Source Files" in Xcode"'
)

add_library(wserialization
  src/basic_text_wiprimitive.cpp
  src/basic_text_woprimitive.cpp
  src/text_wiarchive.cpp
  src/text_woarchive.cpp
  src/utf8_codecvt_facet.cpp
  src/xml_wiarchive.cpp
  src/xml_woarchive.cpp
  src/codecvt_null.cpp
  src/xml_wgrammar.cpp
)

target_include_directories(serialization PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(wserialization PUBLIC ${Boost_INCLUDE_DIRS})

# end library build
###########################

####################
# add headers in IDE

# for serialisation

file(GLOB x 
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/boost/archive/*.hpp"
)
add_custom_target(archive SOURCES ${x})
set_property(TARGET archive PROPERTY FOLDER "serialization")

file(GLOB x 
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/boost/archive/detail/*.hpp"
)
add_custom_target(archive-detail SOURCES ${x})
set_property(TARGET archive-detail PROPERTY FOLDER "serialization")

file(GLOB x 
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/boost/archive/impl/*.ipp"
)
add_custom_target(archive-impl SOURCES ${x})
set_property(TARGET archive-impl PROPERTY FOLDER "serialization")

file(GLOB x
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/boost/archive/iterators/*.hpp"
)
add_custom_target(archive-iterators SOURCES ${x})
set_property(TARGET archive-iterators PROPERTY FOLDER "serialization")

file(GLOB x 
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/boost/serialization/*.hpp"
)
add_custom_target(serialization-headers SOURCES ${x})
set_property(TARGET serialization-headers PROPERTY FOLDER "serialization")

file(GLOB x 
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/boost/serialization/detail/*.hpp"
)
add_custom_target(serialization-detail SOURCES ${x})
set_property(TARGET serialization-detail PROPERTY FOLDER "serialization")

# for wserialization

file(GLOB x 
  RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/boost/archive/*_w*.hpp"
)

add_custom_target(wserialization_headers SOURCES ${x} "${CMAKE_CURRENT_SOURCE_DIR}/../include/boost/archive/codecvt_null.hpp")
set_property(TARGET wserialization_headers PROPERTY FOLDER "wserialization")

# end headers in IDE
####################
