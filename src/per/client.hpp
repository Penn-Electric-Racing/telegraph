#ifndef __PER_CLIENT_HPP__
#define __PER_CLIENT_HPP__

#include <vector>
#include <string>
#include <memory>
#include <rethinkdb.h>

namespace per {
    class tree;
    class client {
    public:
        client(const std::string& host, int port);

        // will give an error if a table with that name
        // already exists
        void add(const std::string& name, tree& tree);

        // will delete a tree (by table)
        // fails silently if tree does not exist
        void remove(const std::string& name);

        void replace(const std::string& name, tree& tree);

        // use forwarding to prevent copying the tree
        tree&& get(const std::string& name);
    private:
        void insert(const std::shared_ptr<node>& n);

        std::shared_ptr<RethinkDB::Connection> conn_;
    };
}
#endif
