#add_library(turnstile_lib turnstile.cpp)
#TODO uncomment
#add_subdirectory(tests)
set(CMAKE_CXX_STANDARD 17)
#add_compile_options(-DWITH_STD_MUTEX)
add_compile_options(-DDEBUG)
add_executable(turnstile_lib main.cpp turnstile.h util.h)
#add_executable(turn1stile_lib sandbox.cpp)