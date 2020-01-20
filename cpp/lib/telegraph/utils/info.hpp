#ifndef __TELEGRAPH_INFO_HPP__
#define __TELEGRAPH_INFO_HPP__

#include <map>
#include <vector>
#include <string>

namespace telegraph {
    class info {
    private:
        enum type { Empty, Number, String, Object, Array };
        type type_;
        union box {
            float number;
            std::string str;
            std::map<std::string, info> object;
            std::vector<info> array;
	    inline box() {}
            ~box() {}
        } value_;
    public:
        inline info() : type_(Empty), value_() {}
        inline info(float num) : type_(Number), value_() { value_.number = num; }
        inline info(const std::string& str) : type_(String), value_() { value_.str = str; }
        inline info(const std::vector<info>& a) : type_(Array), value_() { value_.array = a; }
        inline info(const std::map<std::string, info>& o) : type_(Object), value_() { value_.object = o; }

        inline info(std::string&& str) : type_(String), value_() { value_.str = std::move(str); }
        inline info(std::vector<info>&& a) : type_(Array), value_() { value_.array = std::move(a); } 
        inline info(std::map<std::string, info>&& o) : type_(Object), value_() { value_.object = std::move(o); }

        info(info&& i) : type_(i.type_), value_{} {
            switch(type_) {
            case Empty: break;
            case Number: value_.number = i.value_.number; break;
            case String: value_.str = std::move(i.value_.str); break;
            case Object: value_.object = std::move(i.value_.object); break;
            case Array: value_.array = std::move(i.value_.array); break;
            }
        }

        info(const info& i) : type_(i.type_), value_{} {
            switch(type_) {
            case Empty: break;
            case Number: value_.number = i.value_.number; break;
            case String: value_.str = i.value_.str; break;
            case Object: value_.object = i.value_.object; break;
            case Array: value_.array = i.value_.array; break;
            }
        }

        void operator=(const info& i) {
            switch(type_) {
            case String: value_.str.~basic_string(); break;
            case Object: value_.object.~map<std::string, info>(); break;
            case Array: value_.array.~vector<info>(); break;
            default: break;
            }
            type_ = i.type_;
            switch(type_) {
            case Empty: break;
            case Number: value_.number = i.value_.number; break;
            case String: value_.str = i.value_.str; break;
            case Object: value_.object = i.value_.object; break;
            case Array: value_.array = i.value_.array; break;
            }
        }

        ~info() {
            switch(type_) {
            case String: value_.str.~basic_string(); break;
            case Object: value_.object.~map<std::string, info>(); break;
            case Array: value_.array.~vector<info>(); break;
            default: return;
            }
        }

        constexpr float number() const {
            if (type_ != Number) throw bad_type_error("expected type number");
            return value_.number;
        }
    };
}

#endif
