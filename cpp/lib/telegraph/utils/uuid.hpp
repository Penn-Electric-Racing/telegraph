#ifndef __TELEGRAPH_UUID_HPP__
#define __TELEGRAPH_UUID_HPP__

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

namespace telegraph {
    using uuid = boost::uuids::uuid;
    using random_uuid_generator = boost::uuids::random_generator;

    inline uuid rand_uuid() {
        random_uuid_generator gen;
        return gen();
    }
}

#include <boost/functional/hash.hpp>

namespace std {

template<>
    struct hash<boost::uuids::uuid> {
        size_t operator () (const boost::uuids::uuid& uid) const {
            return boost::hash<boost::uuids::uuid>()(uid);
        }
    };

}

#endif
