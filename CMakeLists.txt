cmake_minimum_required(VERSION 3.15)

project(xDateTime)
set(ABSL_PROPAGATE_CXX_STD ON)

if (DEFINED WITH_ABSEIL)
    add_subdirectory(abseil-cpp)
endif()

add_subdirectory(tests)
