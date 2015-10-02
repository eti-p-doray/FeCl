
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
  @Turbo/Turbo_createPermutation.cpp
  @Ldpc/Ldpc_constructor.cpp
  @Ldpc/Ldpc_get_decoderOptions.cpp
  @Ldpc/Ldpc_set_encoderOptions.cpp
  @Ldpc/Ldpc_set_decoderOptions.cpp
  @Convolutional/Convolutional_constructor.cpp
  @Convolutional/Convolutional_get_decoderOptions.cpp
  @Convolutional/Convolutional_set_encoderOptions.cpp
  @Convolutional/Convolutional_set_decoderOptions.cpp
  @Convolutional/Convolutional_createPermutation.cpp
  @Trellis/Trellis_constructor.cpp
)

if(MATLAB_FOUND)
  add_library(wrap MODULE ${SOURCES} wrap.cpp Matlabdef.def)
  target_include_directories(wrap PRIVATE ${MATLAB_INCLUDE_DIRS})
  target_include_directories(wrap PRIVATE ${Boost_INCLUDE_DIRS})
  target_include_directories(wrap PRIVATE ${CMAKE_SOURCE_DIR}/src)
  target_link_libraries(wrap FeClStatic)
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
endif(MATLAB_FOUND)


