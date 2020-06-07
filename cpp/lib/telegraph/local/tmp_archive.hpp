#ifndef __TELEGRAPH_LOCAL_TMP_ARCHIVE__
#define __TELEGRAPH_LOCAL_TMP_ARCHIVE__

#include "../common/data.hpp"
#include "../common/nodes.hpp"

#include "namespace.hpp"

namespace telegraph {

    class tmp_data : public data_query {
    private:
        std::vector<data_point> current_;
    public:
        const std::vector<data_point>& get_current() const override { return current_; }
        void write(const std::vector<data_point>& d) {
            current_.insert(current_.end(), d.begin(), d.end());
            data(d);
        }
    };
    // for the request recording info
    class tmp_archive : public local_context {
    private:
        std::unordered_map<const variable*, std::shared_ptr<tmp_data>> data_;
        std::unordered_map<const variable*, subscription_ptr> recordings_;
        std::unordered_map<params_stream*, 
            std::weak_ptr<params_stream>> recordings_queries_;
    public:
        tmp_archive(io::io_context& ioc, const std::string_view& name,
                    std::unique_ptr<node>&& s);
        ~tmp_archive();

        params_stream_ptr request(io::yield_ctx&, const params& p) override;

        void record(variable* v, subscription_ptr s);
        void record_stop(variable* v);

        bool write_data(io::yield_ctx& yield, variable* v,
                        const std::vector<data_point>& data) override {
            auto it = data_.find(v);
            if (it == data_.end()) {
                auto s = std::make_shared<tmp_data>();
                data_[v] = s;
                s->write(data);
            } else {
                it->second->write(data);
            }
            return true;
        }
        bool write_data(io::yield_ctx& yield, 
                        const std::vector<std::string_view>& v,
                        const std::vector<data_point>& data) override {
            auto n =  tree_->from_path(v);
            auto var = dynamic_cast<variable*>(n);
            if (!var) return false;
            return write_data(yield, var, data);
        }

        data_query_ptr query_data(io::yield_ctx& ctx,
                                  const variable* v) override {
            auto it = data_.find(v);
            if (it == data_.end()) {
                auto s = std::make_shared<tmp_data>();
                data_[v] = s;
                return s;
            }
            return it->second;
        }
        
        data_query_ptr query_data(io::yield_ctx& ctx,
                                  const std::vector<std::string_view>& v) override;

        subscription_ptr subscribe(io::yield_ctx& ctx,
                const variable* v, 
                float min_interval, float max_interval, 
                float timeout) override { 
            return nullptr;
        }
        subscription_ptr subscribe(io::yield_ctx& yield,
                const std::vector<std::string_view>& path,
                float min_interval, float max_interval, 
                float timeout) override {
            return nullptr;
        }

        value call(io::yield_ctx& yield, action* a, value v, float timeout) override { 
            return value::invalid();
        }

        value call(io::yield_ctx& yield, 
                    const std::vector<std::string_view>& path, 
                    value v, float timeout) override {
            return value::invalid();
        }

        static local_context_ptr create(io::yield_ctx&, io::io_context& ioc, 
                const std::string_view& name, const std::string_view& type,
                const params& p);
    };
}

#endif