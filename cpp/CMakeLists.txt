cmake_minimum_required(VERSION 3.9)

SET (CMAKE_C_COMPILER             "/usr/bin/clang")
SET (CMAKE_CXX_COMPILER           "/usr/bin/clang++")
SET (CMAKE_CXX_FALGS              "-Wall,-fcoroutines-ts")

set(CMAKE_CXX_STANDARD 17)

project(libcom)

list(APPEND CMAKE_PREFIX_PATH "/opt/grpc" "/opt/protobuf")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)

set(PROTOS ${CMAKE_CURRENT_LIST_DIR}/../proto/api.proto)

set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto-src)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})

add_subdirectory(extern)

# cpp-hocon requires boost regex and leatherman
find_package(Boost REQUIRED COMPONENTS regex thread REQUIRED)
find_package(Leatherman COMPONENTS utils locale REQUIRED)

file(GLOB_RECURSE LIBCOM_SOURCES "src/**.cpp")
file(GLOB_RECURSE LIBCOM_HEADERS "src/**.hpp")

add_library(com STATIC ${LIBCOM_SOURCES} ${LIBCOM_HEADERS} ${PROTO_SRCS} ${GRPC_SRCS})

target_include_directories(com
    PUBLIC "src" ${CMAKE_CURRENT_BINARY_DIR}/proto-src)
target_link_libraries(com
    PUBLIC json
    protobuf::libprotobuf gRPC::grpc++ 
    cpp-hocon ${LEATHERMAN_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test "apps/test.cpp")
target_link_libraries(test com)

add_executable(client-test "apps/client-test.cpp")
target_link_libraries(client-test com)
