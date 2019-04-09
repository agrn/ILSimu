cmake_minimum_required(VERSION 2.8)

project(rasseiver CXX)

set(MAJOR_VERSION 0)
set(MINOR_VERSION 0)
set(PACKAGE rasseiver)
set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION})
set(VERSION ${VERSION_STRING})

add_executable(${PACKAGE} main.cpp)
target_link_libraries(${PACKAGE} ${LIBAIRSPY_LIBRARIES})
