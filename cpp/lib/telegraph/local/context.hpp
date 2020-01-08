#ifndef __TELEGRAPH_LOCAL_CONTEXT_HPP__
#define __TELEGRAPH_LOCAL_CONTEXT_HPP__

#include "../common/context.hpp"
#include "../utils/io.hpp"

namespace telegraph {
    class local_namespace;

    class local_context : public context {
    public:
        local_context(const std::string& name, const std::string& type, const info& i);
        void reg(local_namespace* ns);

        std::shared_ptr<node> fetch(io::yield_context) override;

        std::unique_ptr<query<mount_info>>  mounts(io::yield_context, 
                bool srcs=true, bool tgts=true) const override;

        bool destroy(boost::asio::yield_context yield) override;
    protected:
        std::shared_ptr<node> tree_;
        local_namespace* ns_;
    };
}

#endif
