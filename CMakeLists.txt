cmake_minimum_required (VERSION 3.0.0)
project(ecc)

include (GenerateExportHeader)
set(CMAKE_BUILD_TYPE Release)

set(Boost_USE_STATIC_LIBS       ON) # only find static libs
set(Boost_USE_MULTITHREADED    	ON)
set(Boost_USE_STATIC_RUNTIME   	OFF)
find_package(Boost 1.40.0 REQUIRED COMPONENTS serialization)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_library(fec STATIC
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

target_link_libraries(fec PRIVATE ${Boost_LIBRARIES})
target_include_directories(fec PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(fec PUBLIC ${CMAKE_SOURCE_DIR})
install(TARGETS fec ARCHIVE DESTINATION lib)
install(FILES Code.h DESTINATION include/fec )
install(DIRECTORY TurboCode ConvolutionalCode LdpcCode CodeStructure DESTINATION include/fec FILES_MATCHING PATTERN "*.h")

find_package(Doxygen)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
#install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_executable(test_fec test/test_fec.cpp)
target_link_libraries(test_fec fec)
add_test( test_fec test_fec)
target_include_directories(test_fec PRIVATE ${Boost_INCLUDE_DIRS})