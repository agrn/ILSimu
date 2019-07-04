cmake_minimum_required(VERSION 3.7)

project(rasseiver LANGUAGES CXX)

set(MAJOR_VERSION 0)
set(MINOR_VERSION 0)
set(PACKAGE rasseiver)
set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION})
set(VERSION ${VERSION_STRING})

add_executable(${PACKAGE} src/main.cpp src/config.cpp src/device_airspy.cpp
  src/device_dummy.cpp src/device_rspduo.cpp src/filter.cpp src/sender.cpp)
find_library(libsdrplay NAMES libsdrplay_api.so.3.01)
message(STATUS ${libsdrplay})

add_library(sdrplay STATIC IMPORTED)
set_target_properties(sdrplay PROPERTIES IMPORTED_LOCATION ${libsdrplay})

target_link_libraries(${PACKAGE} PUBLIC ${LIBAIRSPY_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT} sdrplay)
