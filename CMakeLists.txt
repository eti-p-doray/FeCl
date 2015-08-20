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
  set(CMAKE_CXX_FLAGS "-std=c++11 -fPIC ${CMAKE_C_FLAGS}")
endif (UNIX)

set(SOURCES
  src/Code.cpp
  src/CodeStructure/CodeStructure.cpp
  src/CodeStructure/ConvolutionalCodeStructure.cpp
  src/CodeStructure/TurboCodeStructure.cpp
  src/CodeStructure/LdpcCodeStructure.cpp
  src/CodeStructure/TrellisStructure.cpp
  src/ConvolutionalCode/ConvolutionalCode.cpp
  src/ConvolutionalCode/MapDecoder/MapDecoder.cpp
  src/ConvolutionalCode/ViterbiDecoder/ViterbiDecoder.cpp
  src/TurboCode/TurboCode.cpp
  src/TurboCode/TurboCodeImpl.cpp
  src/LdpcCode/LdpcCode.cpp
  src/LdpcCode/BpDecoder/BpDecoder.cpp
)

add_library (core OBJECT ${SOURCES})
target_include_directories(core PUBLIC ${Boost_INCLUDE_DIRS})

add_library (fec SHARED $<TARGET_OBJECTS:core>)
target_link_libraries(fec ${Boost_LIBRARIES})
target_include_directories(fec PUBLIC ${Boost_INCLUDE_DIRS})

add_library (fecStatic STATIC $<TARGET_OBJECTS:core>)
target_link_libraries(fecStatic ${Boost_LIBRARIES})
target_include_directories(fecStatic PUBLIC ${Boost_INCLUDE_DIRS})

add_subdirectory(+fec)

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
add_executable(test_code testCpp/code.cpp)
target_include_directories(test_code PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(test_code PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(test_code fecStatic)
target_link_libraries(test_code ${Boost_LIBRARIES})
add_test(code test_code)

add_executable(example_Convolutional exampleCpp/Convolutional.cpp)
target_include_directories(example_Convolutional PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(example_Convolutional PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(example_Convolutional fecStatic)
target_link_libraries(example_Convolutional ${Boost_LIBRARIES})

add_executable(example_Turbo exampleCpp/Turbo.cpp)
target_include_directories(example_Turbo PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(example_Turbo PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(example_Turbo ${Boost_LIBRARIES})
target_link_libraries(example_Turbo fecStatic)

add_executable(example_Ldpc exampleCpp/Ldpc.cpp)
target_include_directories(example_Ldpc PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(example_Ldpc PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(example_Ldpc ${Boost_LIBRARIES})
target_link_libraries(example_Ldpc fecStatic)

