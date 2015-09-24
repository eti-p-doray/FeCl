cmake_minimum_required (VERSION 2.8.11)

set(CPP_TESTS
  Cpp/Convolutional.cpp
  Cpp/Turbo.cpp
  Cpp/Ldpc.cpp
)

foreach(TEST ${CPP_TESTS})
  get_filename_component(TEST_NAME ${TEST} NAME_WE)
  add_executable(test_cpp_${TEST_NAME} ${TEST})
  target_include_directories(test_cpp_${TEST_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
  target_include_directories(test_cpp_${TEST_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_libraries(test_cpp_${TEST_NAME} fecStatic)
  target_link_libraries(test_cpp_${TEST_NAME} ${Boost_LIBRARIES})
  add_test(NAME ${TEST_NAME} COMMAND test_cpp_${TEST_NAME} --log_level=test_suite)
endforeach()