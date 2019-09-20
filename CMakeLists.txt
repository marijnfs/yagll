cmake_minimum_required (VERSION 3.2)
set (CMAKE_CXX_STANDARD 20)

project (yagll)
file(GLOB_RECURSE SRC
    src/*.cc
)
file(GLOB_RECURSE HDR
    *.h
)

include_directories(inc)
include_directories(.)
add_library(yagll SHARED ${SRC})

#install(TARGETS yagll LIBRARY DESTINATION lib)
