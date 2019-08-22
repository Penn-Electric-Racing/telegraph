#ifndef __PER_NODES_STREAM_HPP__
#define __PER_NODES_STREAM_HPP__

namespace per {
    // represents a transmission-controlled
    // raw byte stream to which values can both be written
    // to and read from in bulk

    // this is useful for large data transfer, such
    // as dumping the SD card data, 
    // flushing new code through the bootloader, etc.

    // currently not used, so this is just a placeholder
    class stream : public node {
    };
}
#endif
