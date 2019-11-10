#ifndef __TELEGEN_ID_ARRAY_HPP__
#define __TELEGEN_ID_ARRAY_HPP__

#include <array>

namespace telegen {
    template<size_t N>
        using id_array = std::array<int32_t, N>;
}

#endif
