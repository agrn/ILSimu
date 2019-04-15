cmake_minimum_required(VERSION 3.7)
project(ilsimu)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif ()

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++14")

set(CMAKE_C_FLAGS_DEBUG "-ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O2 -s -flto -march=native")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

find_package(LIBAIRSPY REQUIRED)
find_package(Threads REQUIRED)
include_directories(${LIBAIRSPY_INCLUDE_DIR})

add_subdirectory(rasseiver)
