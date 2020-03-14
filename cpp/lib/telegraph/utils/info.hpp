#ifndef __TELEGRAPH_INFO_HPP__
#define __TELEGRAPH_INFO_HPP__

#include <map>
#include <vector>
#include <string>
#include <variant>

#include "../utils/errors.hpp"

namespace telegraph {

    namespace api {
        class Info;
    }

    class info {
    private:
        std::variant<std::monostate, float, 
            std::string, std::map<std::string, info>,
                std::vector<info>> value_;
    public:
        inline info() : value_() {}
        inline info(float num) : value_(num) {}
        inline info(const std::string& str) : value_(str) {}
        inline info(const std::vector<info>& a) : value_(a) {}
        inline info(const std::map<std::string, info>& o) : value_(o) {}

        inline info(std::string&& str) : value_(std::move(str)) {}
        inline info(std::vector<info>&& a) : value_(std::move(a)) {}
        inline info(std::map<std::string, info>&& o) : value_(std::move(o)) {}

        info(info&& i) : value_(std::move(i.value_)) {}
        info(const info& i) : value_(i.value_) {}

        info(const api::Info& i);

        void operator=(const info& i) {
            value_ = i.value_;
        }
        void operator=(info&& i) {
            value_ = std::move(i.value_);
        }

        constexpr bool is_number() const {
            return std::holds_alternative<float>(value_);
        }
        constexpr float number() const {
            const float* n = std::get_if<float>(&value_);
            if (!n) throw bad_type_error("expected type number");
            return *n;
        }

        void pack(api::Info*) const;
    };
}

#endif
