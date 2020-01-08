#ifndef __TELEGRAPH_CONTEXT_HPP__
#define __TELEGRAPH_CONTEXT_HPP__

#include "../utils/info.hpp"
#include "../utils/uuid.hpp"
#include "../utils/io.hpp"
#include "data.hpp"

#include <memory>
#include <string>

namespace telegraph {
    class namespace_;
    class variable;
    class action;
    class node;

    struct mount_info {
        uuid src_ns;
        uuid src_ctx;
        uuid tgt_ns;
        uuid tgt_ctx;
    };

    class context : public std::enable_shared_from_this<context> {
    public:
        inline context(const uuid& uuid, const std::string& name, 
                const std::string& type, const info& i) : 
                    uuid_(uuid), name_(name),
                    type_(type), info_(i) {}

        const std::string& get_name() const { return name_; }
        const uuid& get_uuid() const { return uuid_; }

        inline virtual std::shared_ptr<node> 
            fetch(io::yield_context ctx) { return nullptr; }

        inline virtual subscription_ptr 
            subscribe(io::yield_context ctx, variable* v, 
                    int32_t min_interval, int32_t max_interval) { return nullptr; }

        inline virtual value call(boost::asio::yield_context ctx, 
                action* a, const value& v) { return value(); }

        inline virtual bool write_data(boost::asio::yield_context yield, node* n, 
                                        const std::vector<data_point>& data) { return false; }

        inline virtual std::unique_ptr<data_query> 
            query_data(boost::asio::yield_context yield, 
                        const node* n) const { return nullptr; }

        inline virtual std::unique_ptr<query<mount_info>> 
            mounts(boost::asio::yield_context ctx, 
                    bool srcs=true, bool tgts=true) const { return nullptr; }

        inline virtual bool mount(boost::asio::yield_context ctx, 
                                const context& src) { return false; }

        inline virtual bool unmount(boost::asio::yield_context ctx, 
                                const context& src) { return false; }

        inline virtual bool destroy(boost::asio::yield_context yield) { return false; }
    private:
        uuid uuid_;
        std::string name_;
        std::string type_;
        info info_;
    };
    using context_ptr = std::shared_ptr<context>;
}

#endif
