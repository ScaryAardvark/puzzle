cmake_minimum_required( VERSION 2.8 )
project( puzzle )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wpedantic -Werror -O6 -fno-exceptions"  )

add_executable( puzzle grid.cpp main.cpp puzzle.cpp wordcache.cpp )
target_link_libraries( puzzle pthread tbb )

add_executable( analyse analyse.cpp )
