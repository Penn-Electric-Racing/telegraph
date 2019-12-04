#ifndef __TELEGRAPH_PORT_HPP__
#define __TELEGRAPH_PORT_HPP__

namespace telegraph {
    class port {
    public:
        port();
        ~port();

        // if tree is null, will fetch the tree from the device when opening
        // the tree will be set to use this port

        // the returned tree is on the heap. the callee will be responsible for its
        // memory management
        tree* open(const std::shared_ptr<std::iostream>& stream, tree* tree=nullptr);
        void close();
    private:
        std::shared_ptr<std::iostream> stream_;
    };
}

#endif
