#ifndef __TELEGRAPH_GEN_TRAITS_HPP__
#define __TELEGRAPH_GEN_TRAITS_HPP__

namespace telegraph::gen { 
    template<typename T>
        class type_traits {
            using storage_type = T;
        };
}
