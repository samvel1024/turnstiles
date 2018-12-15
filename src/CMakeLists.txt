add_library(turnstile_lib turnstile.cpp turnstile.h util.h)
#add_definitions(-DDEBUG)
#add_compile_options(-DWITH_STD_MUTEX)
add_subdirectory(tests)
