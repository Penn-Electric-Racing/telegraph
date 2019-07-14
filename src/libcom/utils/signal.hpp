#ifndef __PER_SIGNAL_HPP__
#define __PER_SIGNAL_HPP__

#include <functional>
#include <vector>

namespace per {
    template<typename... T>
        class signal {
            public:
                signal() : listeners_() {}
                void operator+=(const std::function<void(T...)> &cb) { listeners_.push_back(cb); }
                void operator()(T... v) const { 
                    std::vector<std::function<void(T...)>> ls(listeners_);
                    for (auto& l : ls) {
                        l(v...);
                    }
                }
            private:
                std::vector<std::function<void(T...)>> listeners_;
        };
}

#endif
