set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/) # add FindMatlab module

find_package(Matlab REQUIRED)
set(Boost_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/include/)
set(Boost_LIBRARIES serialization)

set(SOURCES
  @Codec/Codec_destroy.cpp
  @Codec/Codec_save.cpp
  @Codec/Codec_load.cpp
  @Codec/Codec_encode.cpp
  @Codec/Codec_decode.cpp
  @Codec/Codec_soDecode.cpp
  @Codec/Codec_get_msgSize.cpp
  @Codec/Codec_get_systSize.cpp
  @Codec/Codec_get_stateSize.cpp
  @Codec/Codec_get_paritySize.cpp
  @Codec/Codec_get_workGroupSize.cpp
  @Codec/Codec_set_workGroupSize.cpp
  @Turbo/Turbo_constructor.cpp
  @Turbo/Turbo_get_decoderOptions.cpp
  @Turbo/Turbo_set_encoderOptions.cpp
  @Turbo/Turbo_set_decoderOptions.cpp
)

add_library(wrap MODULE ${SOURCES} wrap.cpp Matlabdef.def)
target_include_directories(wrap PRIVATE ${MATLAB_INCLUDE_DIRS})
target_include_directories(wrap PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(wrap PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(wrap fecStatic)
target_link_libraries(wrap ${Boost_LIBRARIES})
target_link_libraries(wrap ${MATLAB_LIBRARIES})

set_target_properties(wrap PROPERTIES CMAKE_CXX_VISIBILITY_PRESET hidden)
set_target_properties(wrap PROPERTIES CMAKE_VISIBILITY_INLINES_HIDDEN 1)
set_target_properties(wrap PROPERTIES SUFFIX .${MATLAB_MEXEXT} PREFIX "")
set_target_properties(wrap PROPERTIES COMPILE_FLAGS -DMATLAB_MEX_FILE)

if (APPLE)
  set_target_properties(wrap PROPERTIES LINK_FLAGS -Wl,-exported_symbols_list,"${MATLAB_MAP}")
elseif (UNIX)
  set_target_properties(wrap PROPERTIES LINK_FLAGS -Wl,--version-script,"${MATLAB_MAP}")
endif (APPLE)

install(TARGETS wrap DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/+bin)

#foreach(SRC_NAME ${SOURCES})
#  get_filename_component(FUNCTION_NAME ${SRC_NAME} NAME_WE)
#
#  add_library(${FUNCTION_NAME} MODULE MexFunction.cpp Matlabdef.def)
#  target_include_directories(${FUNCTION_NAME} PRIVATE ${MATLAB_INCLUDE_DIRS})
#  target_link_libraries(${FUNCTION_NAME} fecMatlab)
#  target_link_libraries(${FUNCTION_NAME} ${MATLAB_LIBRARIES})
#  set_target_properties(${FUNCTION_NAME} PROPERTIES CMAKE_CXX_VISIBILITY_PRESET hidden)
#  set_target_properties(${FUNCTION_NAME} PROPERTIES CMAKE_VISIBILITY_INLINES_HIDDEN 1)
#  set_target_properties(${FUNCTION_NAME} PROPERTIES SUFFIX .${MATLAB_MEXEXT} PREFIX "")
#  set_target_properties(${FUNCTION_NAME} PROPERTIES COMPILE_FLAGS -DMEX_FUNCTION=${FUNCTION_NAME})
#
#  if (APPLE)
#    set_target_properties(${FUNCTION_NAME} PROPERTIES LINK_FLAGS -Wl,-exported_symbols_list,"${MATLAB_MAP}")
#  elseif (UNIX)
#    set_target_properties(${FUNCTION_NAME} PROPERTIES LINK_FLAGS -Wl,--version-script,"${MATLAB_MAP}")
#  endif (APPLE)
#
#  install(TARGETS ${FUNCTION_NAME} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/+bin)
#endforeach()


