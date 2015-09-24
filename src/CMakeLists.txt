set(SOURCES
  Codec.cpp
  Structure/Trellis.cpp
  Convolutional/Convolutional.cpp
  Convolutional/MapDecoder/MapDecoder.cpp
  Convolutional/MapDecoder/MapDecoderImpl.cpp
  Convolutional/ViterbiDecoder/ViterbiDecoder.cpp
  Convolutional/ViterbiDecoder/ViterbiDecoderImpl.cpp
  Turbo/Turbo.cpp
  Turbo/TurboDecoder/TurboDecoder.cpp
  Turbo/TurboDecoder/TurboDecoderImpl.cpp
  Ldpc/Ldpc.cpp
  Ldpc/DvbS2.cpp
  Ldpc/BpDecoder/BpDecoder.cpp
  Ldpc/BpDecoder/BpDecoderImpl.cpp
)

add_library (core OBJECT ${SOURCES})
target_include_directories(core PUBLIC ${Boost_INCLUDE_DIRS})

add_library (fec SHARED $<TARGET_OBJECTS:core>)
target_link_libraries(fec ${Boost_LIBRARIES})
target_include_directories(fec PUBLIC ${Boost_INCLUDE_DIRS})
set_target_properties(fec PROPERTIES LINKER_LANGUAGE CXX)

add_library (fecStatic STATIC $<TARGET_OBJECTS:core>)
target_link_libraries(fecStatic ${Boost_LIBRARIES})
target_include_directories(fecStatic PUBLIC ${Boost_INCLUDE_DIRS})
set_target_properties(fecStatic PROPERTIES LINKER_LANGUAGE CXX)

if (UNIX)
  install(TARGETS fec DESTINATION lib)
  install(TARGETS fecStatic DESTINATION lib)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include/fec FILES_MATCHING PATTERN "*.h")
endif (UNIX)

if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
		${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
endif(DOXYGEN_FOUND)
