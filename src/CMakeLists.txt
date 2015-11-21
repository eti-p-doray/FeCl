set(SOURCES
  Codec.cpp
  Trellis.cpp
  Convolutional.cpp
  Turbo.cpp
  Lte3Gpp.cpp
  Ldpc.cpp
  DvbS2.cpp
  detail/Convolutional.cpp
  detail/MapDecoder/MapDecoder.cpp
  detail/MapDecoder/MapDecoderImpl.cpp
  detail/ViterbiDecoder/ViterbiDecoder.cpp
  detail/ViterbiDecoder/ViterbiDecoderImpl.cpp
  detail/Turbo.cpp
  detail/TurboDecoder/TurboDecoder.cpp
  detail/TurboDecoder/TurboDecoderImpl.cpp
  detail/Ldpc.cpp
  detail/BpDecoder/BpDecoder.cpp
  detail/BpDecoder/BpDecoderImpl.cpp
)

add_library (core OBJECT ${SOURCES})
target_include_directories(core PUBLIC ${Boost_INCLUDE_DIRS})

add_library (FeCl SHARED $<TARGET_OBJECTS:core>)
target_link_libraries(FeCl ${Boost_LIBRARIES})
target_include_directories(FeCl PUBLIC ${Boost_INCLUDE_DIRS})
set_target_properties(FeCl PROPERTIES LINKER_LANGUAGE CXX)

add_library (FeClStatic STATIC $<TARGET_OBJECTS:core>)
target_link_libraries(FeClStatic ${Boost_LIBRARIES})
target_include_directories(FeClStatic PUBLIC ${Boost_INCLUDE_DIRS})
set_target_properties(FeClStatic PROPERTIES LINKER_LANGUAGE CXX)

if (UNIX)
  install(TARGETS FeCl DESTINATION lib)
  install(TARGETS FeClStatic DESTINATION lib)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION include/FeCl FILES_MATCHING PATTERN "*.h")
endif (UNIX)

if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
		${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(docCpp
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
endif(DOXYGEN_FOUND)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Wrap)
