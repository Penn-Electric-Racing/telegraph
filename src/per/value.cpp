#include "value.hpp"

namespace per {
    std::ostream& operator<<(std::ostream& o, const value& v) {
        switch (v.get_type_class()) {
        case type::UINT8: o << (int) v.get<uint8_t>(); break;
        }
        return o;
    }
}
