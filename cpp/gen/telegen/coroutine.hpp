#ifndef __TELEGEN_COROUTINE_HPP__
#define __TELEGEN_COROUTINE_HPP__

// we put the if(__bp) in there to make
// that section dead code that can only
// be reached by explicitly jumping in there
#define TELEGEN_REENTER_IMPL_(coro) \
    switch (::telegen::coroutine::ref __bp = coro) \
        case -1: if (__bp) { \
          goto terminate_reenter; \
          terminate_reenter: \
          __bp = -1; \
          goto exit_reenter; \
          exit_reenter: \
          break; \
        } \
    else case 0: \

#define TELEGEN_AWAIT_IMPL_(n, awaitable) \
    { \
        __bp = n; \
        case n: \
            awaitable.resume(); \
            if (::telegen::coroutine::ref(awaitable) != -1) { \
                goto exit_reenter; \
            } \
    }

#define TELEGEN_AWAIT_WHILE_IMPL_(n, condition) \
    { \
        __bp = n; \
        case n: \
            if (condition) {\
                goto exit_reenter; \
            } \
    }

#define TELEGEN_AWAIT_UNTIL_IMPL_(n, condition) \
    { \
        __bp = n; \
        case n: \
            if (!condition) {\
                goto exit_reenter; \
            } \
    }

#define reenter(coro) TELEGEN_REENTER_IMPL_(coro)

#define await(awaitable) TELEGEN_AWAIT_IMPL_(__COUNTER__ + 1, awaitable)
#define await_while(condition) TELEGEN_AWAIT_WHILE_IMPL_(__COUNTER__ + 1, condition)
#define await_until(condition) TELEGEN_AWAIT_UNTIL_IMPL_(__COUNTER__ + 1, condition)

namespace telegen {
    struct coroutine {
    public:
        class ref {
        public:
            inline ref(coroutine& c) : breakpoint_(c.breakpoint_), modified_(false) {}
            inline ref(coroutine* c) : breakpoint_(c->breakpoint_), modified_(false) {}

            // if we execute without modifying the breakpoint
            // this means we have reached the end of the reenter block
            inline ~ref() { if (!modified_) breakpoint_ = -1; }

            inline operator int() const {
                return breakpoint_;
            }

            inline int& operator=(int bp) { 
                modified_ = true; 
                breakpoint_ = bp;
            }
        private:
            int& breakpoint_;
            bool modified_;
        };
        friend class ref;

        constexpr coroutine() : breakpoint_(0) {}

        constexpr void reset() { breakpoint_ = 0; }
        constexpr void finish() { breakpoint_ = -1; }

        // will run a coroutine until it is done
        inline void run() {
            while (breakpoint_ >= 0) {
                resume();
            }
        }

        virtual void resume() = 0;
    private:
        int breakpoint_;
    };

    struct co_join : public coroutine {
    public:
        void resume() {
        }
    };
}

#endif
