cmake_minimum_required( VERSION 3.2 )
project( puzzle )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wpedantic -Werror -O6 -fno-exceptions"  )

set( SOURCES 
    grid.cpp
    main.cpp
    puzzle.cpp
    wordcache.cpp
)

add_executable( puzzle ${SOURCES} )
target_link_libraries( puzzle pthread tbb )
