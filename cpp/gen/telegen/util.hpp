#ifndef __TELEGRAPH_UTIL_HPP__
#define __TELEGRAPH_UTIL_HPP__

#include "pb_encode.h"
#include <cstdint>

namespace telegen {
    namespace util {
        inline bool proto_string_encoder(pb_ostream_t* stream,
                    const pb_field_iter_t* field, void* const* arg) {
            const char* str = (const char*) *arg;
            if (!pb_encode_tag_for_field(stream, field)) 
                return false;
            return pb_encode_string(stream, (uint8_t*) str, strlen(str));
        }
    }
}

#endif
