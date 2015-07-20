
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/Matlab/cmake) # add FindMatlab module

add_definitions(/DMATLAB_MEX_FILE) #define matlab macros

find_package(Matlab REQUIRED)

function (add_mex_function SRC_NAME)
	get_filename_component(FUNCTION_NAME ${SRC_NAME} NAME_WE)
	add_library(${FUNCTION_NAME} SHARED ${SRC_NAME})
	target_include_directories(${FUNCTION_NAME} PRIVATE ${MATLAB_INCLUDE_DIRS})
	target_include_directories(${FUNCTION_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
	target_link_libraries(${FUNCTION_NAME} PRIVATE ecc)
	target_link_libraries(${FUNCTION_NAME} PRIVATE ${Boost_LIBRARIES})
	target_link_libraries(${FUNCTION_NAME} PRIVATE ${MATLAB_LIBRARIES})
	
	if (UNIX)
		SET_TARGET_PROPERTIES(${FUNCTION_NAME} PROPERTIES LINK_FLAGS -Wl,-exported_symbols_list,"${CMAKE_SOURCE_DIR}/Matlab/mexFunction.map")
	endif (UNIX)

  	set_target_properties(${FUNCTION_NAME} PROPERTIES SUFFIX .${MATLAB_MEXEXT} PREFIX "")
	generate_export_header(${FUNCTION_NAME} BASE_NAME ecc EXPORT_FILE_NAME ${CMAKE_SOURCE_DIR}/Matlab/ecc_export.h)

	install(TARGETS ${FUNCTION_NAME} DESTINATION ${CMAKE_SOURCE_DIR}/Matlab/bin)
endfunction()


add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_appDecode.cpp)
add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_decode.cpp)
add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_destructor.cpp)
add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_encode.cpp)
add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_get_extrinsicMsgSize.cpp)
add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_get_extrinsicParitySize.cpp)
add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_get_msgSize.cpp)
add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_get_paritySize.cpp)
add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_load.cpp)
add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_parityAppDecode.cpp)
add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_save.cpp)
add_mex_function(@ErrorCorrectingCode/ErrorCorrectingCode_softOutDecode.cpp)

add_mex_function(@ConvolutionalCode/ConvolutionalCode_constructor.cpp)
add_mex_function(@TurboCode/TurboCode_constructor.cpp)
add_mex_function(@LdpcCode/LdpcCode_constructor.cpp)


	