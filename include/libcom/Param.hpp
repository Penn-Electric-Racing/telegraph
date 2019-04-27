#ifndef __H_LIBCOM_PARAM_HPP__
#define __H_LIBCOM_PARAM_HPP__

#include "PacketProcessor.hpp"
#include "Error.hpp"

#include <functional>
#include <vector>

namespace libcom {
    template<typename Type>
    class Param {
    public:
        typedef uint32_t Id;
        typedef std::function<void(const Type&)> Handler;

        inline Param(uint32_t id, std::string name="") : 
            _name(name), _id(id), _value(), _requested(),
            _reqHandlers(), _updateHandlers() {}

        // accessors
        inline Id id() const { return _id; }
        inline const std::string& name() const { return _name; }
        inline const Type& value() const { return _value; }
        inline const Type& requested() const { return _requested; }

        // handlers
        inline void requestHandler(const Handler& l) {
            _reqHandlers.push_back(l);
        }
        inline void updateHandler(const Handler& l) {
            _updateHandlers.push_back(l);
        }

        // request operator
        inline void operator<<(const Type& v) {
            _requested = v;

            std::vector<std::function<void(const Type&)>> l(_reqHandlers);
            for (std::function<void(const Type&)>& f : l) {
                f(_requested);
            }
        }

        // update operator
        friend inline void operator>>(const Type& v, Param<Type> &p) {
            p._value = v;

            std::vector<std::function<void(const Type&)>> l(p._updatedHandlers);
            for (std::function<void(const Type&)>& f : l) {
                f(p._value);
            }
        }

    private:
        std::string _name;
        Id          _id;
        Type        _value;
        Type        _requested;
        std::vector<Handler> _reqHandlers;
        std::vector<Handler> _updateHandlers;
    };
    template<typename Type>
        PacketProcessor& operator+=(PacketProcessor& proc,
                                                Param<Type>* p);

    // special type for bool due to size
    template<>
        PacketProcessor& operator+=(PacketProcessor& proc,
                                                Param<bool>* p) {
            proc.add(p->id(), [p] (const Packet& pack) {
                    if (pack.size() != 1)
                        throw ParseError("invalid size for bool param");
                    (*p) << (bool) *((uint8_t*) pack.data());
                });
            return proc;
        }

    template<typename Type>
        PacketProcessor& operator+=(PacketProcessor& proc,
                                                Param<Type>* p) {
            proc.add(p->id(), [p] (const Packet& pack) {
                    if (pack.size() != sizeof(Type))
                        throw ParseError("invalid size for param");
                    (*p) << *((Type*) pack.data());
                });
            return proc;
        }
}

#endif
