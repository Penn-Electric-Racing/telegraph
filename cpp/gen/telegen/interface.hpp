#ifndef __TELEGEN_INTERFACE_HPP__
#define __TELEGEN_INTERFACE_HPP__

namespace telegen {
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
