#ifndef __H_LIBCOM_PARAM_HPP__
#define __H_LIBCOM_PARAM_HPP__

#include "Error.hpp"

#include <string>
#include <functional>
#include <vector>
#include <iostream>

namespace libcom {
    enum class ParamType {
        INVALID,
        BOOL,
        UINT8, UINT16, UINT32, UINT64,
        INT8, INT16, INT32, INT64,
        FLOAT,
        DOUBLE
    };
    // Base class for containing any
    // type of parameter
    class GenericParam {
    public:
        typedef uint32_t Id;
        typedef std::function<void()> Handler;

        inline GenericParam(ParamType type, Id id, std::string name="") : 
                    _name(name), _type(type), _id(id) {}

        // accessors
        inline Id id() const { return _id; }
        inline ParamType type() const { return _type; }
        inline const std::string& name() const { return _name; }

        // to be overridden by the templated type
        virtual void requestHandler(const Handler& l) = 0;
        virtual void updateHandler(const Handler& l, bool always=false) = 0;
        virtual bool update(uint8_t* data, size_t n) = 0;

        // string representation of the parameter value/request for debugging
        virtual std::string valueStr() const = 0;
        virtual std::string requestStr() const = 0;
    private:
        std::string _name;
        ParamType _type;
        Id _id;
    };

    template<typename Type>
    class Param : public GenericParam {
    public:
        typedef std::function<void(const Type&)> ValueHandler;

        constexpr static ParamType type = std::is_same<Type, bool>::value ? ParamType::BOOL :
                                std::is_same<Type, uint8_t>::value ? ParamType::UINT8 :
                                std::is_same<Type, uint16_t>::value ? ParamType::UINT16 :
                                std::is_same<Type, uint32_t>::value ? ParamType::UINT32 :
                                std::is_same<Type, uint64_t>::value ? ParamType::UINT64 :
                                std::is_same<Type, int8_t>::value ? ParamType::INT8 :
                                std::is_same<Type, int16_t>::value ? ParamType::INT16 :
                                std::is_same<Type, int32_t>::value ? ParamType::INT32 :
                                std::is_same<Type, int64_t>::value ? ParamType::INT64 :
                                std::is_same<Type, float>::value ? ParamType::FLOAT :
                                std::is_same<Type, double>::value ? ParamType::DOUBLE : 
                                ParamType::INVALID;

        Param(Id id, std::string name="") : 
            GenericParam(type, id, name), _value(), _requested(),
            _reqHandlers(), _updateHandlers() {}

        const Type& value() const { return _value; }
        const Type& requested() const { return _requested; }

        // emtpy handlers
        void requestHandler(const Handler& l) override {
            _reqHandlers.push_back([l] (const Type& t) { l(); });
        }
        void updateHandler(const Handler& l, bool always=false) override {
            if (always) _updateAlwaysHandlers.push_back([l] (const Type& t) { l(); });
            else _updateHandlers.push_back([l] (const Type& t) { l(); });
        }

        void requestHandler(const ValueHandler& l) {
            _reqHandlers.push_back(l);
        }
        void updateHandler(const ValueHandler& l, bool always=false) {
            if (always) _updateAlwaysHandlers.push_back(l);
            else _updateHandlers.push_back(l);
        }

        void request(const Type& v) {
            _requested = v;

            std::vector<std::function<void(const Type&)>> l(_reqHandlers);
            for (std::function<void(const Type&)>& f : l) {
                f(_requested);
            }
        }

        bool update(const Type& v) {
            bool same = _value == v;
            _value = v;

            std::vector<std::function<void(const Type&)>> al(_updateAlwaysHandlers);
            for (std::function<void(const Type&)>& f : al) {
                f(v);
            }

            if (same) return false; // do nothing we this is already the value

            std::vector<std::function<void(const Type&)>> l(_updateHandlers);
            for (std::function<void(const Type&)>& f : l) {
                f(v);
            }
            return true;
        }

        // for updating using a buffer
        bool update(uint8_t* data, size_t n) override {
            if (std::is_same<Type, bool>::value) {
                if (n != 1) throw ParseError("invalid size for bool param");
                bool val = (bool) (*((uint8_t*) data) & 0x1);
                return update(val);
            } else {
                if (n != sizeof(Type)) throw ParseError("invalid size for param");
                return update(*((Type*) data));
            }
        }
        
        // request operator shortcut
        bool operator<<(const Type& v) {
            return request(v);
        }

        // debugging string values
        std::string valueStr() const override {
            if (std::is_same<Type, bool>::value) {
                return _value == 0 ? "false" : "true";
            } else {
                return std::to_string(_value);
            }
        }

        std::string requestStr() const override {
            if (std::is_same<Type, bool>::value) {
                return _requested == 0 ? "false" : "true";
            } else {
                return std::to_string(_requested);
            }
        }
    private:
        Type        _value;
        Type        _requested;
        std::vector<ValueHandler> _reqHandlers;
        std::vector<ValueHandler> _updateHandlers;
        std::vector<ValueHandler> _updateAlwaysHandlers;
    };

}

#endif
