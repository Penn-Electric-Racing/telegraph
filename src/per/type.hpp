#ifndef __PER_TYPE_HPP__
#define __PER_TYPE_HPP__

#include <vector>
#include <string>

namespace per {
    class type {
    public:
        enum type_class {
            INVALID, NONE, ENUM,
            BOOL,
            UINT8, UINT16, UINT32, UINT64,
            INT8, INT16, INT32, INT64,
            FLOAT, DOUBLE
        };
        inline type() : class_(INVALID), name_(), labels_() {}
        inline type(type_class c) : class_(c), name_(), labels_() {}

        constexpr type_class get_class() const { return class_; }
        constexpr const std::string& get_name() const { return name_; }

        inline void add_label(const std::string& label) { labels_.push_back(label); }
        inline void set_labels(const std::vector<std::string> labels) { labels_ = labels; }

        inline void set_name(const std::string& name) { name_ = name; }

        std::string to_str() const;
    private:
        type_class class_;
        std::string name_;
        // contains the unit for this type
        // for for an enum the labels per value
        std::vector<std::string> labels_;
    };
}

#endif
