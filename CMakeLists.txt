cmake_minimum_required (VERSION 3.0.0)
project(fec LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)

set(Boost_USE_STATIC_LIBS       ON) # only find static libs
set(Boost_USE_STATIC_RUNTIME   	OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/serialization)
set(Boost_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/include/)
set(Boost_LIBRARIES serialization)
add_definitions(-DBOOST_ALL_NO_LIB)

if (UNIX)
  set(CMAKE_CXX_FLAGS " -std=c++11 -fPIC ${CMAKE_C_FLAGS}")
endif (UNIX)

set(SOURCES
  src/Codec.cpp
  src/Structure/Trellis.cpp
  src/Convolutional/Convolutional.cpp
  src/Convolutional/MapDecoder/MapDecoder.cpp
  src/Convolutional/MapDecoder/MapDecoderImpl.cpp
  src/Convolutional/ViterbiDecoder/ViterbiDecoder.cpp
  src/Convolutional/ViterbiDecoder/ViterbiDecoderImpl.cpp
  src/Turbo/Turbo.cpp
  src/Turbo/TurboDecoder/TurboDecoder.cpp
  src/Turbo/TurboDecoder/TurboDecoderImpl.cpp
  src/Ldpc/Ldpc.cpp
  src/Ldpc/BpDecoder/BpDecoder.cpp
  src/Ldpc/BpDecoder/BpDecoderImpl.cpp
)

add_library (core OBJECT ${SOURCES})
target_include_directories(core PUBLIC ${Boost_INCLUDE_DIRS})

add_library (fec SHARED $<TARGET_OBJECTS:core>)
target_link_libraries(fec ${Boost_LIBRARIES})
target_include_directories(fec PUBLIC ${Boost_INCLUDE_DIRS})

add_library (fecStatic STATIC $<TARGET_OBJECTS:core>)
target_link_libraries(fecStatic ${Boost_LIBRARIES})
target_include_directories(fecStatic PUBLIC ${Boost_INCLUDE_DIRS})

#add_subdirectory(+fec)

if (UNIX)
  install(TARGETS fec DESTINATION lib)
  install(TARGETS fecStatic DESTINATION lib)
  install(FILES src/Code.h DESTINATION include/fec )
  install(DIRECTORY TurboCode/src ConvolutionalCode/src LdpcCode/src CodeStructure/src DESTINATION include/fec FILES_MATCHING PATTERN "*.h")
endif (UNIX)

find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Doxyfile
		${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
endif(DOXYGEN_FOUND)

enable_testing()

set(TESTS
  testCpp/Convolutional.cpp
  testCpp/Turbo.cpp
)

foreach(TEST ${TESTS})
  get_filename_component(TEST_NAME ${TEST} NAME_WE)
  add_executable(test_${TEST_NAME} ${TEST})
  target_include_directories(test_${TEST_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
  target_include_directories(test_${TEST_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_libraries(test_${TEST_NAME} fecStatic)
  target_link_libraries(test_${TEST_NAME} ${Boost_LIBRARIES})
  add_test(${TEST_NAME} test_${TEST_NAME})
endforeach()



