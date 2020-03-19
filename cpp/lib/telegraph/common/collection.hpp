#ifndef __TELEGRAPH_COLLECTION_HPP__
#define __TELEGRAPH_COLLECTION_HPP__

#include "../utils/uuid.hpp"
#include "../utils/signal.hpp"
#include "../utils/io_fwd.hpp"
#include "../utils/errors.hpp"

#include <memory>
#include <functional>
#include <unordered_map>

namespace telegraph {
    // specialize this to use special
    // unique key types (such as uuid)
    // that can be quickly retrieved/filtered by
    template<typename T>
        struct collection_key {
            typedef T type;

            static type get(const T& v) { return v; }
        };

    template<typename T>
        class collection : public std::enable_shared_from_this<collection<T>> {
        private:
            std::shared_ptr<collection> src_;
            std::function<bool(const T&)> filter_;
        public:
            using key = typename collection_key<T>::type;

            // public signals/map
            signal<const T&> added;
            signal<const T&> removed;

            std::unordered_map<key, T> current;

            collection() {}
            collection(const std::shared_ptr<collection>& src,
                    const std::function<bool(const T&)>& filter) 
                    : src_(src), filter_(filter), added(), removed() {
                for (auto& v : src->current) {
                    if (filter(v.second)) current.insert(v);
                }
                src_->added.add(this, [this] (const T& v) { add_(v); });
                src_->removed.add(this, [this] (const T& v) { remove_(v); });
            }

            // has to be a shared_ptr so that you can call chain()
            // on the resulting collection
            std::shared_ptr<collection> filter(const std::function<bool(const T&)>& f) {
                // since the base class is dependant on the template parameter,
                // we need to do a full name lookup
                std::shared_ptr<collection> q = 
                    std::enable_shared_from_this<collection<T>>::shared_from_this();
                return std::make_shared<collection>(q, f);
            }

            bool has(const key& k) const {
                return current.find(k) != current.end();
            }

            constexpr size_t size() const {
                return current.size();
            }

            const T &result() const {
                if (current.size() != 1)
                    throw missing_error("must have exactly one result to do get");
                return current.begin()->second;
            }

            T get(const key& k) const {
                auto it = current.find(k);
                if (it == current.end()) return T();
                return it->second;
            }

            // public, but should not be called unless you
            // know what you are doing
            void add_(const T& v) {
                if (filter_ && !filter_(v)) return;
                auto k = collection_key<T>::get(v);

                if (has(k)) return;
                current.insert(std::make_pair(k, v));
                added(v);
            }

            void remove_(const T& v) {
                if (filter_ && !filter_(v)) return;
                auto k = collection_key<T>::get(v);
                auto it = current.find(k);
                if (it == current.end()) return;
                current.erase(it);
                removed(v);
            }

            void remove_by_key_(const key& k) {
                auto it = current.find(k);
                if (it == current.end()) return;
                remove_(it->second);
            }

            typename std::unordered_map<key, T>::iterator begin() { return current.begin(); }
            typename std::unordered_map<key, T>::iterator begin() const { return current.begin(); }
            typename std::unordered_map<key, T>::iterator end() { return current.end(); }
            typename std::unordered_map<key, T>::iterator end() const { return current.end(); }
        };

    template<typename T>
        using collection_ptr = std::shared_ptr<collection<T>>;
}

#endif
