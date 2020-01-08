#ifndef __TELEGEN_PUBLISHER_HPP__
#define __TELEGEN_PUBLISHER_HPP__

namespace telegen {
    template<typename T, typename Clock>
        class publisher {
        public:
            publisher(Clock& c) : clock_(c);

            std::unique_ptr<subscription> subscribe(variable_base* v,
                        int32_t min_inteval, int32_t max_interval) {
                return std::unique_ptr<subscription>();
            }
        private:
            Clock& clock_;
        };
}

#endif
