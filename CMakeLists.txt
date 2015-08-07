cmake_minimum_required (VERSION 3.0.0)
project(ecc)

include (GenerateExportHeader)
set(CMAKE_BUILD_TYPE Release)

set(Boost_USE_STATIC_LIBS       ON) # only find static libs
set(Boost_USE_MULTITHREADED    	ON)
set(Boost_USE_STATIC_RUNTIME   	OFF)
find_package(Boost 1.40.0 REQUIRED COMPONENTS serialization)

set (CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_library(ecc STATIC
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

target_link_libraries(ecc PRIVATE ${Boost_LIBRARIES})
target_include_directories(ecc PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(ecc PUBLIC ${CMAKE_SOURCE_DIR})

enable_testing()

add_executable(test_ecc test/test_ecc.cpp)
target_link_libraries(test_ecc ecc)

add_test( test_ecc test_ecc)
target_include_directories(test_ecc PRIVATE ${Boost_INCLUDE_DIRS})

add_subdirectory(Matlab)