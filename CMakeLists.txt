cmake_minimum_required(VERSION 3.9)

set(LIBCOM_SOURCES
    "src/libcom/SerialPort.cpp"
    "src/libcom/Packet.cpp"
    "src/libcom/PacketProcessor.cpp"
    "src/libcom/Msg.cpp"
    )
set(LIBCOM_HEADERS
    "include/libcom/Param.hpp"
    "include/libcom/SerialPort.hpp"
    "include/libcom/Packet.hpp"
    "include/libcom/PacketProcessor.hpp"
    "include/libcom/Msg.hpp"
    )

add_library(libcom SHARED ${LIBCOM_SOURCES} ${LIBCOM_HEADERS})

target_include_directories(libcom
    PRIVATE 
        "include/libcom")
target_include_directories(libcom
    PUBLIC
        "include")


# Add tests
add_executable(serialize-test "src/tests/serialize.cpp")
target_link_libraries(serialize-test libcom)

add_executable(port-test "src/tests/port.cpp")
target_link_libraries(port-test libcom)
