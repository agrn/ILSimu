cmake_minimum_required(VERSION 3.7)
project(ilsimu)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif ()

set(CMAKE_C_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "-std=gnu++14")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -ggdb")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O2 -s -flto -march=native")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${CMAKE_CXX_FLAGS}")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

find_package(LIBAIRSPY REQUIRED)
include_directories(${LIBAIRSPY_INCLUDE_DIR})

add_subdirectory(rasseiver)
