#ifndef __TELEGRAPH_GEN_TRAITS_HPP__
#define __TELEGRAPH_GEN_TRAITS_HPP__

namespace telegen { 
    template<typename T>
        struct storage_traits {
            using type = T;
        };
}

#endif