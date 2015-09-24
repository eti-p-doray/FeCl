cmake_minimum_required (VERSION 2.8.11)

set(BENCHMARK_CPP
  Cpp/main.cpp
)

add_custom_target(benchmark
  COMMENT "Execute benchmark tests" VERBATIM
)

if(ITPP_FOUND)
  add_executable(benchmark_cpp ${BENCHMARK_CPP})
  target_include_directories(benchmark_cpp PUBLIC ${CMAKE_SOURCE_DIR}/src)
  target_include_directories(benchmark_cpp PUBLIC ${Boost_INCLUDE_DIRS})
  target_include_directories(benchmark_cpp PUBLIC ${ITPP_INCLUDE_DIRS})
  target_link_libraries(benchmark_cpp fecStatic)
  target_link_libraries(benchmark_cpp ${Boost_LIBRARIES})
  target_link_libraries(benchmark_cpp ${ITPP_LIBRARIES})

  add_custom_command(TARGET benchmark POST_BUILD
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/benchmark_cpp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compare fec with itpp" VERBATIM
  )
endif(ITPP_FOUND)

if(MATLAB_FOUND)
  add_custom_command(
    TARGET benchmark POST_BUILD
    COMMAND ${MATLAB_BIN} -nosplash -nodesktop -r main
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Matlab
    COMMENT "Compare fec with itpp" VERBATIM
  )
endif(MATLAB_FOUND)

