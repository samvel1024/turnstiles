add_executable(trivial_test trivial_test.cpp)
target_link_libraries(trivial_test turnstile_lib)
target_link_libraries(trivial_test "${CMAKE_THREAD_LIBS_INIT}")
add_test(trivial_test trivial_test)


add_executable(nested_mutex_test nested_mutex_test.cpp)
#add_compile_options(-DWITH_STD_MUTEX)
target_link_libraries(nested_mutex_test turnstile_lib)
target_link_libraries(nested_mutex_test "${CMAKE_THREAD_LIBS_INIT}")
add_test(nested_mutex_test nested_mutex_test)
