#ifndef __TELEGRAPH_UTIL_HPP__
#define __TELEGRAPH_UTIL_HPP__

#include "pb_encode.h"
#include <cstdint>
#include <tuple>
#include <utility>
#include <functional>

namespace telegen {
    namespace util {
        inline bool proto_string_encoder(pb_ostream_t* stream,
                    const pb_field_iter_t* field, void* const* arg) {
            const char* str = (const char*) *arg;
            if (!pb_encode_tag_for_field(stream, field)) 
                return false;
            return pb_encode_string(stream, (uint8_t*) str, strlen(str));
        }


        // for packing and unpacking values
        // from tuples/variadic template arguments
        // https://stackoverflow.com/questions/687490/how-do-i-
        // expand-a-tuple-into-variadic-template-functions-arguments

        template<int...> 
            struct index_tuple{}; 

        template<int I, typename IndexTuple, typename... Types> 
            struct make_indexes_impl; 

        template<int I, int... Indexes, typename T, typename ... Types> 
            struct make_indexes_impl<I, index_tuple<Indexes...>, T, Types...>  { 
                typedef typename make_indexes_impl<I + 1, 
                        index_tuple<Indexes..., I>, Types...>::type type; 
            }; 

        template<int I, int... Indexes> 
            struct make_indexes_impl<I, index_tuple<Indexes...>>  { 
                typedef index_tuple<Indexes...> type; 
            }; 

        template<typename ... Types> 
            struct make_indexes : make_indexes_impl<0, index_tuple<>, Types...> {};

        template<class A, class... Args, int... Indexes> 
            void apply_helper(const std::function<void(A, Args&&...)>& f, A a,
                            index_tuple< Indexes... >, std::tuple<Args...>&& tup) { 
                f(a, std::forward<Args>(std::get<Indexes>(tup))... ); 
            } 

        template<class Ret, class A, class ... Args> 
            Ret apply(const std::function<Ret(A, Args&&...)>& f, A a, std::tuple<Args...>&&  tup) {
                return apply_helper(f, a, typename make_indexes<Args...>::type(), 
                        std::forward<std::tuple<Args...>>(tup));
            }
    }
}

#endif
