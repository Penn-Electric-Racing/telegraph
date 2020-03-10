#ifndef __TELEGRAPH_SIGNAL_HPP__
#define __TELEGRAPH_SIGNAL_HPP__

#include <functional>
#include <mutex>
#include <map>

namespace telegraph {
    template<typename... T>
        class signal {
            public:
                signal() : listeners_() {}
                signal<T...>& add(const std::function<void(T...)> &cb) { 
                    listeners_[(void*)&cb] = cb;
                    return *this;
                }
                /**
                 * If you use this, the lambda will have to be removed using ptr
                 */
                signal<T...>& add(void* ptr, const std::function<void(T...)> &cb) {
                    listeners_[ptr] = cb;
                    return *this;
                }

                signal<T...>& remove(const std::function<void(T...)> &cb) {
                    listeners_.erase(&cb);
                    return *this;
                }
                signal<T...>& remove(void* ptr) {
                    listeners_.erase(ptr);
                    return *this;
                }

                void operator()(T&&... v) const { 
                    // by maintaining an iterator ahead of the current
                    // one you can safely remove the listener currently
                    // beign called while we iterate through the map
                    for (auto it = listeners_.cbegin(), next_it = it; 
                            it != listeners_.cend(); it = next_it) {
                        ++next_it;
                        (it->second)(std::forward<T>(v)...);
                    }
                }
            private:
                std::map<void*, std::function<void(T...)>> listeners_;
        };

    /**
     * A threadsafe variant of signal<T>
     */
    /*
    template<typename... T>
        class safe_signal {
            public:
                safe_signal() : listeners_() {}
                safe_signal<T...>& add(const std::function<void(T...)> &cb) { 
                    listeners_[(void*)&cb] = cb;
                    return *this;
                }
                safe_signal<T...>& add(void* ptr, const std::function<void(T...)> &cb) {
                    listeners_[ptr] = cb;
                    return *this;
                }

                safe_signal<T...>& remove(const std::function<void(T...)> &cb) {
                    listeners_.erase(&cb);
                    return *this;
                }
                safe_signal<T...>& remove(void* ptr) {
                    listeners_.erase(ptr);
                    return *this;
                }

                void operator()(T... v) const { 
                    std::map<void*, std::function<void(T...)>> ls(listeners_);
                    for (auto& l : ls) {
                        l.second(v...);
                    }
                }
            private:
                std::map<void*, std::function<void(T...)>> listeners_;
                std::mutex mutex_;
        };
    */
}

#endif
