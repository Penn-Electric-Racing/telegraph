#ifndef __WIRE_PROMISE_HPP__
#define __WIRE_PROMISE_HPP__

#include <functional>
#include <tuple>
#include "util.hpp"
#include "coroutine.hpp"
#include "inplace_function.hpp"

namespace wire {
    enum class promise_status { Waiting, Resolved, Rejected };

    // a promise completer
    // is tied to the promise created from it
    // call complete(), resolve(), or reject()
    // to complete the promise
    template<size_t Cap, typename... T>
        class basic_promise_completer {
        public:
            using func = stdext::inplace_function<void(promise_status, T&&...), Cap>;
        private:
            func ch_;
        public:
            basic_promise_completer() : ch_() {}
            ~basic_promise_completer() {}

            basic_promise_completer(const basic_promise_completer<Cap, T...>& c) = delete;
            basic_promise_completer(basic_promise_completer<Cap, T...>&& c) = delete;
            basic_promise_completer<Cap, T...>& operator=(const basic_promise_completer<Cap, T...>& c) = delete;
            basic_promise_completer<Cap, T...>& operator=(basic_promise_completer<Cap, T...>&& c) = delete;

            void complete(promise_status s, T&&... v) {
                ch_(s, std::forward<T>(v)...);
            }
            void reject(T&&... v) {
                complete(promise_status::Rejected, std::forward<T>(v)...);
            }
            void resolve(T&&... v) {
                complete(promise_status::Resolved, std::forward<T>(v)...);
            }

            void set(const func& ch) {
                ch_ = ch;
            }

            const func& get() const {
                return ch_;
            }

            void clear() {
                ch_ = func();
            }
        };

    template<typename... T>
        using promise_completer = basic_promise_completer<24, T...>;

    template<typename... T>
        using spromise_completer = basic_promise_completer<2*sizeof(void*), T...>;

    template<size_t Cap, typename... T>
        class basic_promise : public coroutine {
            template<typename A>
                using chain_func = stdext::inplace_function<A(promise_status, T&&...), sizeof(void*)>;
            // a callback which contains another promise
            // completer in it, used for chaining promises
            template<typename A>
            struct chainer {
                spromise_completer<A> completer;
                chain_func<A> transform;

                chainer(const chainer& c)
                    : completer(), transform(std::move(c.transform)) {}
                chainer(const chain_func<A>& f)
                    : completer(), transform(f) {}

                void operator()(promise_status s, T&&... v) {
                    completer.complete(s, transform(s, std::forward<T>(v)...));
                }
            };

        public:
            using func = typename basic_promise_completer<Cap, T...>::func;

            basic_promise(promise_status s) : status_(s),
                                    val_(), obj_(nullptr) {}
            basic_promise(T&&... val) : status_(promise_status::Resolved), 
                        val_(std::forward<T>(val)...), obj_(nullptr) {}

            basic_promise(basic_promise_completer<Cap, T...>* obj) 
                : status_(promise_status::Waiting), 
                  val_(), obj_(obj) {

                if (obj_) {
                    obj_->set([this](promise_status s, T&&... val) {
                        receive(s, std::forward<T>(val)...); 
                    });
                }
            }

            basic_promise(basic_promise<Cap, T...>&& p)
                : status_(p.status_), val_(std::move(p.val_)), obj_(p.obj_) {
                p.obj_ = nullptr;

                if (obj_) {
                    obj_->set([this](promise_status s, T&&... val) {
                        receive(s, std::forward(val)...); 
                    });
                }
            }

            basic_promise<Cap, T...>& operator=(basic_promise<Cap, T...>&& p) {
                status_ = p.status;
                val_ = std::move(p.val_);
                obj_= p.obj_;
                p.obj_ = nullptr;

                if (obj_) {
                    obj_->set([this](promise_status s, T&&... val) {
                        receive(s, std::forward(val)...); 
                    });
                }
                return *this;
            }

            ~basic_promise() {
                if (obj_) obj_->clear();
            }

            // disable copy construction and assignment
            basic_promise(const basic_promise<Cap, T...>&p) = delete;
            basic_promise<Cap, T...> &operator=(const basic_promise<Cap, T...>&p) = delete;

            constexpr promise_status status() const { return status_; }
            constexpr bool resolved() const { return status_ == promise_status::Resolved; }
            constexpr bool rejected() const { return status_ == promise_status::Rejected; }

            template<size_t I=0>
                constexpr const typename std::tuple_element<I,std::tuple<T...>>::type& 
                        get() const {
                    return std::get<I>(val_);
                }

            void resume() override {
                if (status_ != promise_status::Waiting) finish();
            }

            // once you use then, the callback takes over the promise
            // and this promise becomes rejecting
            inline void then(const func& cb) {
                if (obj_) {
                    obj_->set(cb);
                    obj_ = nullptr;
                } else if (status_ != promise_status::Waiting) {
                    util::apply(cb, status_, std::move(val_));
                }
            }

            template<typename A>
                basic_promise<8, A> chain(const chain_func<A>& transform) {
                    if (status_ != promise_status::Waiting) 
                        return basic_promise<8, A>(
                                util::apply(transform, status_, std::move(val_)));
                    if (obj_) {
                        obj_->set(chainer<A>(transform));
                        // get the location of the stored chainer
                        // back from the inplace_function
                        chainer<A>* c = reinterpret_cast<chainer<A>*>(obj_->get().data());
                        obj_ = nullptr;
                        return basic_promise<8, A>(&c->completer);
                    } else return basic_promise<8, A>(promise_status::Rejected);
                }
        private:
            void receive(promise_status s, T&&... val) {
                status_ = s;
                val_ = std::tuple(std::forward<T>(val)...);
                obj_ = nullptr;
            }

            promise_status status_;
            std::tuple<T...> val_;
            basic_promise_completer<Cap, T...>* obj_;
        };

    template<typename... T>
        using promise = basic_promise<24, T...>;

    template<typename... T>
        using spromise = basic_promise<8, T...>;
}

#endif
