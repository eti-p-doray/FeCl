set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/) # add FindMatlab module

find_package(Matlab REQUIRED)
add_definitions(-DMATLAB_MEX_FILE) #define matlab macros

set(Boost_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/include/)
set(Boost_LIBRARIES serialization)

function (add_mex_function SRC_NAME)
	get_filename_component(FUNCTION_NAME ${SRC_NAME} NAME_WE)
	add_library(${FUNCTION_NAME} SHARED ${SRC_NAME} Matlabdef.def)
	target_include_directories(${FUNCTION_NAME} PRIVATE ${MATLAB_INCLUDE_DIRS})
	target_include_directories(${FUNCTION_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
 	target_include_directories(${FUNCTION_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
	target_link_libraries(${FUNCTION_NAME} fecStatic)
	target_link_libraries(${FUNCTION_NAME} ${Boost_LIBRARIES})
	target_link_libraries(${FUNCTION_NAME} ${MATLAB_LIBRARIES})

	set_target_properties(${FUNCTION_NAME} PROPERTIES CMAKE_CXX_VISIBILITY_PRESET hidden)
	set_target_properties(${FUNCTION_NAME} PROPERTIES CMAKE_VISIBILITY_INLINES_HIDDEN 1)
  set_target_properties(${FUNCTION_NAME} PROPERTIES SUFFIX .${MATLAB_MEXEXT} PREFIX "")

  if (APPLE)
    SET_TARGET_PROPERTIES(${FUNCTION_NAME} PROPERTIES LINK_FLAGS -Wl,-exported_symbols_list,"${MATLAB_MAP}")
  elseif (UNIX)
    SET_TARGET_PROPERTIES(${FUNCTION_NAME} PROPERTIES LINK_FLAGS -Wl,--version-script,"${MATLAB_MAP}")
  endif (APPLE)

	install(TARGETS ${FUNCTION_NAME} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/+bin)
endfunction()


add_mex_function(@Code/Code_softOutDecode.cpp)
add_mex_function(@Code/Code_appDecode.cpp)
add_mex_function(@Code/Code_decode.cpp)
add_mex_function(@Code/Code_encode.cpp)
add_mex_function(@Code/Code_get_extrinsicSize.cpp)
add_mex_function(@Code/Code_get_msgSize.cpp)
add_mex_function(@Code/Code_get_paritySize.cpp)
add_mex_function(@Code/Code_get_workGroupSize.cpp)
add_mex_function(@Code/Code_set_workGroupSize.cpp)
add_mex_function(@Code/Code_load.cpp)
add_mex_function(@Code/Code_save.cpp)
add_mex_function(@Code/Code_destructor.cpp)

add_mex_function(@ConvolutionalCode/ConvolutionalCode_constructor.cpp)
add_mex_function(@TurboCode/TurboCode_constructor.cpp)
add_mex_function(@TurboCode/TurboCode_get_iterationCount.cpp)
add_mex_function(@TurboCode/TurboCode_set_iterationCount.cpp)
add_mex_function(@LdpcCode/LdpcCode_constructor.cpp)
add_mex_function(@LdpcCode/LdpcCode_get_iterationCount.cpp)
add_mex_function(@LdpcCode/LdpcCode_set_iterationCount.cpp)