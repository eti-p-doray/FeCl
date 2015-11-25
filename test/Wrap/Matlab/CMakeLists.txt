cmake_minimum_required (VERSION 2.8.11)

add_library(MatlabTestWrap MODULE testWrap.cpp Matlabdef.def)
target_include_directories(MatlabTestWrap PRIVATE ${CMAKE_SOURCE_DIR}/Wrap/Matlab)
target_include_directories(MatlabTestWrap PRIVATE ${MATLAB_INCLUDE_DIRS})
target_include_directories(MatlabTestWrap PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(MatlabTestWrap PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(MatlabTestWrap FeClStatic)
target_link_libraries(MatlabTestWrap ${Boost_LIBRARIES})
target_link_libraries(MatlabTestWrap ${MATLAB_LIBRARIES})

set_target_properties(MatlabTestWrap PROPERTIES CMAKE_CXX_VISIBILITY_PRESET hidden)
set_target_properties(MatlabTestWrap PROPERTIES CMAKE_VISIBILITY_INLINES_HIDDEN 1)
set_target_properties(MatlabTestWrap PROPERTIES SUFFIX .${MATLAB_MEXEXT} PREFIX "" OUTPUT_NAME testWrap)
set_target_properties(MatlabTestWrap PROPERTIES COMPILE_FLAGS -DMATLAB_MEX_FILE)

if (APPLE)
  set_target_properties(MatlabTestWrap PROPERTIES LINK_FLAGS -Wl,-exported_symbols_list,"${MATLAB_MAP}")
elseif (UNIX)
  set_target_properties(MatlabTestWrap PROPERTIES LINK_FLAGS -Wl,--version-script,"${MATLAB_MAP}")
endif (APPLE)

install(TARGETS MatlabTestWrap DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/)

add_test(NAME MatlabWrap COMMAND ${MATLAB_BIN} -nosplash -nodesktop -r main WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)




