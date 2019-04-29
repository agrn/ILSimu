cmake_minimum_required(VERSION 3.7)
project(ilsimu)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif ()

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++14")

set(CMAKE_C_FLAGS_DEBUG "-Og -ggdb3")
set(CMAKE_C_FLAGS_RELEASE "-O2 -s -flto -march=native")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -ggdb3 -flto -march=native")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -s -flto -march=native")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

find_package(LIBAIRSPY REQUIRED)
find_package(Threads REQUIRED)
include_directories(${LIBAIRSPY_INCLUDE_DIR})

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
 set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
 set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif ()

add_subdirectory(rasseiver)
