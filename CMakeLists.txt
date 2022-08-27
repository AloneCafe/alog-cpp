cmake_minimum_required (VERSION 3.8)
project ("alog")

set(CMAKE_CXX_STANDARD 20)

add_executable (alog  "alog.hpp" "test.cpp")

message(STATUS "Building on ${CMAKE_SYSTEM}")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(alog pthread)
endif()
