cmake_minimum_required (VERSION 3.0.0)
project(fec LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Release)

set(Boost_USE_STATIC_LIBS       ON) # only find static libs
set(Boost_USE_STATIC_RUNTIME   	OFF)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/serialization)
set(Boost_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/include/)
set(Boost_LIBRARIES serialization)

message(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "-std=c++11 -fPIC ${CMAKE_C_FLAGS}")

set(SOURCES
  Code.cpp
  CodeStructure/CodeStructure.cpp
  CodeStructure/ConvolutionalCodeStructure.cpp
  CodeStructure/TurboCodeStructure.cpp
  CodeStructure/LdpcCodeStructure.cpp
  CodeStructure/TrellisStructure.cpp
  ConvolutionalCode/ConvolutionalCode.cpp
  ConvolutionalCode/MapDecoder/MapDecoder.cpp
  ConvolutionalCode/ViterbiDecoder/ViterbiDecoder.cpp
  TurboCode/TurboCode.cpp
  TurboCode/TurboCodeImpl.cpp
  LdpcCode/LdpcCode.cpp
  LdpcCode/BpDecoder/BpDecoder.cpp
)

add_library (core OBJECT ${SOURCES})
target_include_directories(core PUBLIC ${Boost_INCLUDE_DIRS})

#add_library (fec SHARED $<TARGET_OBJECTS:core>)
add_library (fec STATIC $<TARGET_OBJECTS:core>)

#target_link_libraries(fec PRIVATE ${Boost_LIBRARIES})
#target_include_directories(fec PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(fec PRIVATE ${Boost_LIBRARIES})
target_include_directories(fec PUBLIC ${Boost_INCLUDE_DIRS})

#install(TARGETS fec LIBRARY DESTINATION lib)
install(TARGETS fec ARCHIVE DESTINATION lib)
install(FILES Code.h DESTINATION include/fec )
install(DIRECTORY TurboCode ConvolutionalCode LdpcCode CodeStructure DESTINATION include/fec FILES_MATCHING PATTERN "*.h")

add_subdirectory(+fec)

find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
		${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
endif(DOXYGEN_FOUND)

enable_testing()
add_executable(test_code test/code.cpp)
target_include_directories(test_code PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(test_code fec)
add_test(code test_code)

add_executable(example_Convolutional example/Convolutional.cpp)
target_include_directories(example_Convolutional PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(example_Convolutional fec)

add_executable(example_Turbo example/Turbo.cpp)
target_include_directories(example_Turbo PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(example_Turbo fec)

add_executable(example_Ldpc example/Ldpc.cpp)
target_include_directories(example_Ldpc PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(example_Ldpc fec)

