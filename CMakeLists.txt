cmake_minimum_required (VERSION 3.1.0)

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost 1.36.0 REQUIRED COMPONENTS serialization)

add_library(ErrorCorrectingCode SHARED
	ErrorCorrectingCode.cpp
	CodeStructure/CodeStructure.cpp
	CodeStructure/ConvolutionalCodeStructure.cpp
	CodeStructure/TurboCodeStructure.cpp
	CodeStructure/LdpcCodeStructure.cpp
	CodeStructure/TrellisStructure.cpp
	ConvolutionalCode/ConvolutionalCode.cpp
	ConvolutionalCode/MapDecoder/MapDecoder.cpp
	ConvolutionalCode/ViterbiDecoder/ViterbiDecoder.cpp
	TurboCode/TurboCode.cpp
	LdpcCode/LdpcCode.cpp
	LdpcCode/BpDecoder/BpDecoder.cpp
)

target_link_libraries(ErrorCorrectingCode ${Boost_LIBRARIES})
target_include_directories(ErrorCorrectingCode PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(ErrorCorrectingCode PUBLIC ${CMAKE_SOURCE_DIR})
target_compile_features(ErrorCorrectingCode PUBLIC cxx_template_template_parameters)
target_compile_features(ErrorCorrectingCode PUBLIC cxx_aggregate_default_initializers)

add_subdirectory(Matlab)

enable_testing()

add_executable(test_ErrorCorrectingCode test/test_ErrorCorrectingCode.cpp)
target_link_libraries(test_ErrorCorrectingCode ErrorCorrectingCode)

add_test( test_ErrorCorrectingCode test_ErrorCorrectingCode)