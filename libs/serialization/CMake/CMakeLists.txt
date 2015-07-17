# CMake build control file for Serialization Library tests

cmake_minimum_required(VERSION 3.0)



#
# Compiler settings
#

message(STATUS "compiler is ${CMAKE_CXX_COMPILER_ID}" )

if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  add_definitions( -ftemplate-depth=300 )
  # we use gcc to test for C++03 compatibility
  add_definitions( std=c++03 )
  message(STATUS "compiler is g++ c++03")
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )
  add_definitions( /wd4996 )
  message(STATUS "compiler is MSVC")
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth=300")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0" )
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O3" )
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -dead_strip")
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


###########################
# library builds

add_library(serialization STATIC
  ../src/archive_exception.cpp
  ../src/basic_archive.cpp
  ../src/basic_iarchive.cpp
  ../src/basic_iserializer.cpp
  ../src/basic_oarchive.cpp
  ../src/basic_oserializer.cpp
  ../src/basic_pointer_iserializer.cpp
  ../src/basic_pointer_oserializer.cpp
  ../src/basic_serializer_map.cpp
  ../src/basic_text_iprimitive.cpp
  ../src/basic_text_oprimitive.cpp
  ../src/basic_xml_archive.cpp
  ../src/binary_iarchive.cpp
  ../src/binary_oarchive.cpp
  ../src/codecvt_null.cpp
  ../src/extended_type_info_no_rtti.cpp
  ../src/extended_type_info_typeid.cpp
  ../src/extended_type_info.cpp
  ../src/polymorphic_iarchive.cpp
  ../src/polymorphic_oarchive.cpp
  ../src/stl_port.cpp
  ../src/text_iarchive.cpp
  ../src/text_oarchive.cpp
  ../src/text_wiarchive.cpp
  ../src/text_woarchive.cpp
  ../src/utf8_codecvt_facet.cpp
  ../src/void_cast.cpp
  ../src/xml_archive_exception.cpp
  ../src/xml_iarchive.cpp
  ../src/xml_oarchive.cpp
  ../src/xml_grammar.cpp
  # ../src/basic_xml_grammar.ipp # doesn't show up in "Source Files" in Xcode"'
)
target_include_directories(serialization PRIVATE ${CMAKE_SOURCE_DIR}/libs/include/)

add_library(wserialization STATIC
  ../src/basic_text_wiprimitive.cpp
  ../src/basic_text_woprimitive.cpp
  ../src/text_wiarchive.cpp
  ../src/text_woarchive.cpp
  ../src/utf8_codecvt_facet.cpp
  ../src/xml_wiarchive.cpp
  ../src/xml_woarchive.cpp
  ../src/codecvt_null.cpp
  ../src/xml_wgrammar.cpp
)
target_include_directories(wserialization PRIVATE ${CMAKE_SOURCE_DIR}/libs/include/)

# end library build
###########################


