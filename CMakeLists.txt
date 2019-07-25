cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

project(libcom)

list(APPEND CMAKE_PREFIX_PATH "/opt/grpc" "/opt/protobuf")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_subdirectory(extern)

# cpp-hocon requires boost regex and leatherman
find_package(Boost REQUIRED COMPONENTS regex REQUIRED)
find_package(Leatherman COMPONENTS utils locale REQUIRED)

file(GLOB_RECURSE LIBCOM_SOURCES "src/**.cpp")
file(GLOB_RECURSE LIBCOM_HEADERS "src/**.hpp")

add_library(com SHARED ${LIBCOM_SOURCES} ${LIBCOM_HEADERS})
target_include_directories(com
    PUBLIC "src")
target_link_libraries(com
    PUBLIC json rethinkdb++
    cpp-hocon ${LEATHERMAN_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test "apps/test.cpp")
target_link_libraries(test com)

add_executable(client-test "apps/client-test.cpp")
target_link_libraries(client-test com)
