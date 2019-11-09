#ifndef __TELEGRAPH_GEN_INTERFACE_HPP__
#define __TELEGRAPH_GEN_INTERFACE_HPP__

namespace telegraph::gen {
    template<typename T>
        class variable;
    template<typename T>
        class subscription;


    class generic_interface {
        virtual ~generic_interface() = default;
    };

    template<typename T>
        class interface : public virtual generic_interface {
            virtual void subscribed(variable<T>* v, subscription<T>* s) = 0;
        };
}

#endif
