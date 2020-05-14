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

    class params {
    private:
        std::variant<std::monostate, float, bool,
            std::string, std::map<std::string, params, std::less<>>,
                std::vector<params>> value_;
    public:
        params() : value_() {}
        params(float num) : value_(num) {}
        params(int num) : value_((float) num) {}
        params(bool b) : value_(b) {}
        params(const std::string& str) : value_(str) {}
        params(const std::vector<params>& a) : value_(a) {}
        params(const std::map<std::string, params, std::less<>>& o) : value_(o) {}

        params(std::string&& str) : value_(std::move(str)) {}
        params(std::vector<params>&& a) : value_(std::move(a)) {}
        params(std::map<std::string, params, std::less<>>&& o) : value_(std::move(o)) {}

        params(params&& i) : value_(std::move(i.value_)) {}
        params(const params& i) : value_(i.value_) {}

        params(const api::Params& i);

        void operator=(const params& i) {
            value_ = i.value_;
        }
        void operator=(params&& i) {
            value_ = std::move(i.value_);
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

        void pack(api::Params*) const;
        void move(api::Params*);
    };

    class params_stream {
    private:
        bool closed_;
        std::function<void(params&& p)> handler_;
        std::function<void()> on_close_;
        std::vector<params> queued_; // back-queue if handler not set
    public:
        params_stream() : 
            closed_(false), handler_(), 
            on_close_(), queued_() {}
        ~params_stream() { close(); }

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
    using params_stream_ptr = std::unique_ptr<params_stream>;
}

#endif
