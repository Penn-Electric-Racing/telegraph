#ifndef __TELEGRAPH_PARAMS_HPP__
#define __TELEGRAPH_PARAMS_HPP__

#include <map>
#include <vector>
#include <string>
#include <variant>
#include <memory>

#include "../utils/errors.hpp"
#include "../utils/signal.hpp"
#include "../utils/io_fwd.hpp"

namespace telegraph {

    namespace api {
        class Params;
    }
    class node;
    class namespace_;
    class context;
    class component;

    class params {
    private:
        std::variant<std::monostate, float, bool,
            std::string, std::map<std::string, params, std::less<>>,
                std::vector<params>,  std::shared_ptr<node>, 
                std::shared_ptr<context>> value_;
    public:
        params() : value_() {}
        params(float num) : value_(num) {}
        params(int num) : value_((float) num) {}
        params(bool b) : value_(b) {}
        params(const std::string& str) : value_(str) {}
        params(const std::string_view& str) : value_(std::string{str}) {}
        params(const std::vector<params>& a) : value_(a) {}
        params(const std::map<std::string, params, std::less<>>& o) : value_(o) {}
        params(const std::shared_ptr<node>& n) : value_(n) {}
        params(const std::shared_ptr<context>& ctx) : value_(ctx) {}

        params(std::string&& str) : value_(std::move(str)) {}
        params(std::vector<params>&& a) : value_(std::move(a)) {}
        params(std::map<std::string, params, std::less<>>&& o) : value_(std::move(o)) {}

        params(const std::vector<std::string>& s) : value_(std::vector<params>{}) {
            auto& v = std::get<std::vector<params>>(value_);
            for (const auto& st : s) v.push_back(st);
        }
        params(const std::vector<std::string_view>& s) : value_(std::vector<params>{}) {
            auto& v = std::get<std::vector<params>>(value_);
            for (const auto& st : s) v.push_back(st);
        }

        params(params&& i) : value_(std::move(i.value_)) {}
        params(const params& i) : value_(i.value_) {}


        void operator=(const params& i) {
            value_ = i.value_;
        }
        void operator=(params&& i) {
            value_ = std::move(i.value_);
        }

        static params array() { return params(std::vector<params>{}); }
        static params object() { return params(std::map<std::string, params, std::less<>>{}); }

        template<typename T>
            bool has() const {
                return std::holds_alternative<T>(value_);
            }

        template<typename T>
            T& get() {
                return std::get<T>(value_);
            }
        template<typename T>
            const T& get() const {
                return std::get<T>(value_);
            }

        params& at(const std::string_view& s) {
            auto& v = std::get<std::map<std::string, params, std::less<>>>(value_);
            auto it = v.find(s);
            if (it == v.end()) throw missing_error("missing params key");
            return it->second;
        }

        const params& at(const std::string_view& s) const {
            auto& v = std::get<std::map<std::string, params, std::less<>>>(value_);
            auto it = v.find(s);
            if (it == v.end()) throw missing_error("missing params key");
            return it->second;
        }

        params& operator[](const std::string_view& s) {
            auto& v = std::get<std::map<std::string, params, std::less<>>>(value_);
            auto it = v.find(s);
            if (it == v.end()) {
                v.emplace(s, params{});
                return v.find(s)->second;
            }
            return it->second;
        }

        void push(params&& p) {
            auto& v = std::get<std::vector<params>>(value_);
            v.push_back(std::move(p));
        }
        void push(const params& p) {
            auto& v = std::get<std::vector<params>>(value_);
            v.push_back(p);
        }
        void push(const std::string_view& s) {
            auto& v = std::get<std::vector<params>>(value_);
            v.push_back(params{s});
        }

        bool is_none() const { return value_.index() == 0; }
        bool is_num() const { return value_.index() == 1; }
        bool is_bool() const { return value_.index() == 2; }
        bool is_str() const { return value_.index() == 3; }
        bool is_object() const { return value_.index() == 4; }
        bool is_array() const { return value_.index() == 5; }
        bool is_tree() const { return value_.index() == 6; }
        bool is_ctx() const { return value_.index() == 7; }

        const std::map<std::string, params, std::less<>>& to_map() const {
            return std::get<std::map<std::string, 
                        params, std::less<>>>(value_);
        }
        const std::vector<params>& to_vector() const {
            return std::get<std::vector<params>>(value_);
        }

        const std::shared_ptr<node>& to_tree() const {
            return std::get<std::shared_ptr<node>>(value_);
        }
        const std::shared_ptr<context>& to_ctx() const {
            return std::get<std::shared_ptr<context>>(value_);
        }

        void pack(api::Params*) const;
        void move(api::Params*);
        static params unpack(const api::Params& i, namespace_* n=nullptr);
    };

    class params_stream {
    private:
        bool closed_;
        std::function<void(params&& p)> handler_;
        std::function<void()> on_close_;
        std::vector<params> queued_; // back-queue if handler not set
    public:
        signal<> destroyed;
        params_stream() : 
            closed_(false), handler_(), 
            on_close_(), queued_() {}
        ~params_stream() { 
            close(); 
            destroyed();
        }

        constexpr bool is_closed() const { return closed_; }

        void close() {
            if (!closed_) {
                closed_ = true;
                if (on_close_) on_close_();
            }
        }

        void write(params&& p) {
            if (closed_) return;
            if (handler_) handler_(std::move(p));
            else queued_.emplace_back(std::move(p));
        }

        void reset_pipe() {
            handler_ = std::function<void(params&&)>{};
            on_close_ = std::function<void()>{};
        }

        void set_pipe(const std::function<void(params&& p)>& h,
                      const std::function<void()>& on_close) {
            handler_ = h;
            on_close_ = on_close;
            for (auto& p : queued_) {
                h(std::move(p));
            }
            queued_.clear();
            if (closed_) on_close_();
        }
    };
    using params_stream_ptr = std::shared_ptr<params_stream>;
}

#endif
