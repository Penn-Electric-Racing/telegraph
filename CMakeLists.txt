cmake_minimum_required(VERSION 3.9)

set(LIBCOM_SOURCES
    "src/libcom/Packet.cpp"
    "src/libcom/Connection.cpp"
    "src/libcom/SerialPort.cpp"
    "src/libcom/Msg.cpp"
    "src/libcom/XML.cpp"
    )
set(LIBCOM_HEADERS
    "include/libcom/Param.hpp"
    "include/libcom/Packet.hpp"
    "include/libcom/Connection.hpp"
    "include/libcom/SerialPort.hpp"
    "include/libcom/Msg.hpp"
    "include/libcom/XML.hpp"
    )

add_library(com SHARED ${LIBCOM_SOURCES} ${LIBCOM_HEADERS})

target_include_directories(com
    PRIVATE 
        "include/libcom")
target_include_directories(com
    PUBLIC
        "include")

target_link_libraries(com
    PUBLIC pugixml)


# Add tests
add_executable(serialize-test "src/tests/serialize.cpp")
target_link_libraries(serialize-test com)

add_executable(port-test "src/tests/port.cpp")
target_link_libraries(port-test com)
