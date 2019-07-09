cmake_minimum_required(VERSION 3.9)

project(libcom)

file(GLOB_RECURSE LIBCOM_SOURCES "src/**.cpp")
file(GLOB_RECURSE LIBCOM_HEADERS "src/**.hpp")

add_library(libcom SHARED ${LIBCOM_SOURCES} ${LIBCOM_HEADERS})
target_include_directories(libcom
    PUBLIC
        "src")
target_link_libraries(libcom
    PUBLIC pugixml)

add_executable(test "apps/test.cpp")
target_link_libraries(test libcom)
