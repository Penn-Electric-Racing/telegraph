#ifndef __TELEGEN_PROMISE_HPP__
#define __TELEGEN_PROMISE_HPP__

#include <functional>
#include <tuple>
#include "util.hpp"
#include "coroutine.hpp"

namespace telegen {
    enum class promise_status { Waiting, Resolved, Rejected };

    // a promise completer
    // is tied to the returned promise
    // call complete(), resolve(), or reject()
    template<typename... T>
    class promise_completer {
    private:
        // completion handlers
        void* chained_; // a pointer to another promise_completer
                        // allocated on the heap
                        // that will transform the returned value
                        // before reaching the end promise
        std::function<void(promise_status, T&&...)> ch_;
        bool completed_;
    public:
        promise_completer() : chained_(nullptr), ch_() {}
        ~promise_completer() {
            if (chained_) delete chained_;
        }

        promise_completer(const promise_completer<T...>& c) = delete;
        promise_completer(promise_completer<T...>&& c) = delete;
        promise_completer<T...>& operator=(const promise_completer<T...>& c) = delete;
        promise_completer<T...>& operator=(promise_completer<T...>&& c) = delete;

        void complete(promise_status s, T&&... v) {
            ch_(s, std::forward<T>(v)...);
        }
        void reject(T&&... v) {
            complete(promise_status::Rejected, std::forward<T>(v)...);
        }
        void resolve(T&&... v) {
            complete(promise_status::Resolved, std::forward<T>(v)...);
        }

        // NOTE: Chain involves two mallocs -> one for the lambda 
        // and one for the new promise_completer, and should be avoided if possible
        template<typename A>
            promise_completer<A> chain(const std::function<A(promise_status, T&&...)>& c) {
                if (chained_) return nullptr;
                promise_completer<A>* p = new promise_completer<A>();
                chained_ = (void*) p;
                ch_ = [p, c] (promise_status s, T&&... t) {
                    A a = c(s, t...);
                    p->complete(s, std::move(a));
                };
                return p;
            }

        void set(const std::function<void(promise_status, T&&...)>& ch) {
            ch_ = ch;
        }

        void clear() {
            ch_ = std::function<void(promise_status, T&& ...)>();
        }
    };


    template<typename... T>
    class promise : public coroutine {
    public:
        promise(promise_status s) : status_(s),
                                val_(), obj_(nullptr) {}
        promise(T&&... val) : status_(promise_status::Resolved), 
                    val_(std::forward<T>(val)...), obj_(nullptr) {}

        promise(promise_completer<T...>* obj) 
            : status_(promise_status::Waiting), 
              val_(), obj_(obj) {

            if (obj_) {
                obj_->set([this](promise_status s, T&&... val) {
                    handle(s, std::forward(val)...); 
                });
            }
        }

        promise(promise<T...>&& p)
            : status_(p.status_), val_(std::move(p.val_)), obj_(p.obj_) {
            p.obj_ = nullptr;

            if (obj_) {
                obj_->set([this](promise_status s, T&&... val) {
                    handle(s, std::forward(val)...); 
                });
            }
        }

        promise<T...>& operator=(promise<T...>&& p) {
            status_ = p.status;
            val_ = std::move(p.val_);
            obj_= p.obj_;
            p.obj_ = nullptr;

            if (obj_) {
                obj_->set([this](promise_status s, T&&... val) {
                    handle(s, std::forward(val)...); 
                });
            }
            return *this;
        }

        ~promise() {
            if (obj_) obj_->clear();
        }

        // disable copy construction and assignment
        promise(const promise<T...>&p) = delete;
        promise<T...> &operator=(const promise<T...>&p) = delete;

        constexpr promise_status status() const { return status_; }
        constexpr bool resolved() const { return status_ == promise_status::Resolved; }
        constexpr bool rejected() const { return status_ == promise_status::Rejected; }

        template<size_t I=0>
            constexpr const typename std::tuple_element<I,std::tuple<T...>>::type& 
                    get() const {
                return std::get<I>(val_);
            }

        // once you use then, the callback takes over the promise
        // and this promise becomes rejecting
        inline void then(const std::function<void(promise_status, T&&...)>& cb) {
            if (obj_) {
                obj_->set(cb);
                obj_ = nullptr;
            } else if (status_ != promise_status::Waiting) {
                util::apply(cb, status_, std::move(val_));
            }
        }

        // NOTE: chain involves malloc, avoid if possible!
        template<typename A>
            promise<A> chain(const std::function<A(promise_status, T&&...)>& c) {
                if (status_ != promise_status::Waiting) 
                    return promise<A>(c(status_, val_));
                if (obj_) {
                    promise_completer<T...>* o = obj_;
                    obj_ = nullptr;
                    return promise<A>(o->chain(c));
                } else return promise<A>();
            }
    private:
        void handle(promise_status s, T&&... val) {
            status_ = s;
            val_ = std::tuple(std::forward<T>(val)...);
            obj_ = nullptr;
        }
        promise_status status_;
        std::tuple<T...> val_;
        promise_completer<T...>* obj_;
    };
}

#endif
