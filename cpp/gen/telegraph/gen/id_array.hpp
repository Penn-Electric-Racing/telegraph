#ifndef __TELEGRAPH_GEN_ID_ARRAY_HPP__
#define __TELEGRAPH_GEN_ID_ARRAY_HPP__

#include <array>

namespace telegraph::gen {
    template<size_t N>
        using id_array = std::array<int32_t, N>;
}

#endif
