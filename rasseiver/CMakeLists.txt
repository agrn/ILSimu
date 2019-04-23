cmake_minimum_required(VERSION 3.7)

project(rasseiver LANGUAGES CXX)

set(MAJOR_VERSION 0)
set(MINOR_VERSION 0)
set(PACKAGE rasseiver)
set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION})
set(VERSION ${VERSION_STRING})

add_executable(${PACKAGE} main.cpp config.cpp device_airspy.cpp filter.cpp
  sender.cpp)
target_link_libraries(${PACKAGE} PUBLIC ${LIBAIRSPY_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT})
