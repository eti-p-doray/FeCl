
if(MATLAB_FOUND)
  add_library(MatlabWrap MODULE wrap.cpp Matlabdef.def)
  target_include_directories(MatlabWrap PRIVATE ${MATLAB_INCLUDE_DIRS})
  target_include_directories(MatlabWrap PRIVATE ${Boost_INCLUDE_DIRS})
  target_include_directories(MatlabWrap PRIVATE ${CMAKE_SOURCE_DIR}/src)
  target_link_libraries(MatlabWrap FeClStatic)
  target_link_libraries(MatlabWrap ${Boost_LIBRARIES})
  target_link_libraries(MatlabWrap ${MATLAB_LIBRARIES})

  set_target_properties(MatlabWrap PROPERTIES CMAKE_CXX_VISIBILITY_PRESET hidden)
  set_target_properties(MatlabWrap PROPERTIES CMAKE_VISIBILITY_INLINES_HIDDEN 1)
  set_target_properties(MatlabWrap PROPERTIES SUFFIX .${MATLAB_MEXEXT} PREFIX "" OUTPUT_NAME wrap)
  set_target_properties(MatlabWrap PROPERTIES COMPILE_FLAGS -DMATLAB_MEX_FILE)

  if (APPLE)
    set_target_properties(MatlabWrap PROPERTIES LINK_FLAGS -Wl,-exported_symbols_list,"${MATLAB_MAP}")
  elseif (UNIX)
    set_target_properties(MatlabWrap PROPERTIES LINK_FLAGS -Wl,--version-script,"${MATLAB_MAP}")
  endif (APPLE)

  install(TARGETS MatlabWrap DESTINATION ${CMAKE_SOURCE_DIR}/Matlab/+fec/+bin)
endif(MATLAB_FOUND)


