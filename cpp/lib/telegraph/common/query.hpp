#ifndef __TELEGRAPH_QUERY_HPP__
#define __TELEGRAPH_QUERY_HPP__

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
        struct query_key {
            typedef T type;

            static type get(const T& v) { return v; }
        };

    template<typename T>
        class query : public std::enable_shared_from_this<query<T>> {
        private:
            std::shared_ptr<query> src_;
            std::function<bool(const T&)> filter_;
        public:
            using key = typename query_key<T>::type;

            // public signals/map
            signal<io::yield_ctx&, const T&> added;
            signal<io::yield_ctx&, const T&> removed;
            std::unordered_map<key, T> current;

            query() {}
            query(const std::shared_ptr<query>& src,
                    const std::function<bool(const T&)>& filter) 
                    : src_(src), filter_(filter), added(), removed() {
                for (auto& v : src->current) {
                    if (filter(v.second)) current.insert(v);
                }
                src_->added.add(this, [this] (io::yield_ctx& c, const T& v) { add_(c, v); });
                src_->removed.add(this, [this] (io::yield_ctx& c, const T& v) { remove_(c, v); });
            }

            virtual ~query() {
                if (src_) {
                    src_->added.remove(this);
                    src_->removed.remove(this);
                }
            }

            // has to be a shared_ptr so that you can call chain()
            // on the resulting query
            std::shared_ptr<query> chain(const std::function<bool(const T&)>& filter) {
                // since the base class is dependant on the template parameter,
                // we need to do a full name lookup
                std::shared_ptr<query> q = 
                    std::enable_shared_from_this<query<T>>::shared_from_this();
                return std::make_shared<query>(q, filter);
            }

            bool has(const key& k) const {
                return current.find(k) != current.end();
            }

            constexpr size_t size() const {
                return current.size();
            }

            const T &result() const {
                if (current.size() != 1)
                    throw query_error("must have exactly one result to do get");
                return current.begin()->second;
            }

            T get(const key& k) const {
                if (!has(k)) return T();
                return current.at(k);
            }

            // public, but should not be called unless you
            // know what you are doing
            void add_(io::yield_ctx& yield, const T& v) {
                if (filter_ && !filter_(v)) return;
                auto k = query_key<T>::get(v);
                if (has(k)) return;
                current.insert(std::make_pair(k, v));
                added(yield, v);
            }

            void remove_(io::yield_ctx& yield, const T& v) {
                if (filter_ && !filter_(v)) return;
                auto k = query_key<T>::get(v);
                if (!has(k)) return;
                current.erase(query_key<T>::get(v));
                removed(yield, v);
            }

            void remove_by_key_(io::yield_ctx& yield, const key& k) {
                if (!has(k)) return;
                const T& v = current.at(k);
                remove_(yield, v);
            }

            typename std::unordered_map<key, T>::iterator begin() { return current.begin(); }
            typename std::unordered_map<key, T>::iterator begin() const { return current.begin(); }
            typename std::unordered_map<key, T>::iterator end() { return current.end(); }
            typename std::unordered_map<key, T>::iterator end() const { return current.end(); }
        };

    template<typename T>
        using query_ptr = std::shared_ptr<query<T>>;
}

#endif
