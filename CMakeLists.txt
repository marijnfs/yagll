cmake_minimum_required (VERSION 3.2)
set (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (yagll)
file(GLOB_RECURSE SRC
    src/*.cc
)
file(GLOB_RECURSE HDR
    *.h
)

find_library (RE2 NAMES re2 REQUIRED HINTS /usr/lib64)


include_directories(inc)
include_directories(.)
add_library(yagll SHARED ${SRC})
add_executable(parse bin/parse.cc)
target_link_libraries(parse yagll stdc++fs ${RE2})
