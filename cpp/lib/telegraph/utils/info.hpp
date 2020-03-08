#ifndef __TELEGRAPH_INFO_HPP__
#define __TELEGRAPH_INFO_HPP__

#include <map>
#include <vector>
#include <string>
#include <variant>

namespace telegraph {
    class info {
    private:
        enum type { Empty, Number, String, Object, Array };
        type type_;
        std::variant<std::monostate, float, 
            std::string, std::map<std::string, info>,
                std::vector<info>> value_;
    public:
        inline info() : type_(Empty), value_() {}
        inline info(float num) : type_(Number), value_(num) {}
        inline info(const std::string& str) : type_(String), value_(str) {}
        inline info(const std::vector<info>& a) : type_(Array), value_(a) {}
        inline info(const std::map<std::string, info>& o) : type_(Object), value_(o) {}

        inline info(std::string&& str) : type_(String), value_(str) {}
        inline info(std::vector<info>&& a) : type_(Array), value_(a) {}
        inline info(std::map<std::string, info>&& o) : type_(Object), value_(o) {}

        info(info&& i) : type_(i.type_), value_(std::move(i.value_)) {}
        info(const info& i) : type_(i.type_), value_(i.value_) {}

        void operator=(const info& i) {
            type_ = i.type_;
            value_ = std::move(i.value_);
        }

        constexpr float number() const {
            if (type_ != Number) throw bad_type_error("expected type number");
            return std::get<1>(value_);
        }
    };
}

#endif
