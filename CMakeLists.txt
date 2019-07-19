cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

project(libcom)

list(APPEND CMAKE_PREFIX_PATH "/opt/grpc" "/opt/protobuf")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_subdirectory(extern)

# cpp-hocon requires boost regex and leatherman
find_package(Boost REQUIRED COMPONENTS regex REQUIRED)
find_package(Leatherman COMPONENTS utils locale REQUIRED)

find_package(Protobuf REQUIRED)
find_package(GRPC REQUIRED)

set(PROTOS ${CMAKE_CURRENT_LIST_DIR}/api/api.proto)

set(PROTO_SRC_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto-src)
file(MAKE_DIRECTORY ${PROTO_SRC_DIR})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_SRC_DIR} ${PROTOS})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_SRC_DIR} ${PROTOS})

file(GLOB_RECURSE LIBCOM_SOURCES "src/**.cpp")
file(GLOB_RECURSE LIBCOM_HEADERS "src/**.hpp")

add_library(com SHARED ${LIBCOM_SOURCES} ${LIBCOM_HEADERS} ${PROTO_SRCS} ${GRPC_SRCS})
target_include_directories(com
    PUBLIC "src" PRIVATE ${PROTO_SRC_DIR})
target_link_libraries(com
    PUBLIC json 
    protobuf::libprotobuf gRPC::grpc++_reflection
    cpp-hocon ${LEATHERMAN_LIBRARIES} ${Boost_LIBRARIES})

add_executable(test "apps/test.cpp")
target_link_libraries(test com)

add_executable(rpc-test "apps/rpc-test.cpp")
target_link_libraries(rpc-test com)
