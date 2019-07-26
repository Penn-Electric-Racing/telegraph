#include "client.hpp"
#include "errors.hpp"

#include <iostream>


namespace R = RethinkDB;


/*
 * RethinkDB database format notes
 *
 * Every log has its own database with multiple tables:
 *     1) "nodes"
 *          this table has an entry for every node in the tree
 *     2) "subscriptions"
 *          people put subscription requests in here, linking the node
 *          and any subscription details they want. Data from the subscription
 *          will be put into the subscription object in a queue
 *     4) "actions"
 *          people put action requests into here, linking the node they want. 
 *          when the action is fulfilled 
 *          the server puts the result into the action
 *     3) "log"
 *          this is where logged variable/action data is dumped into. 
 *          graphing utilities should listen to this
 */

namespace per {
    client::client(const std::string& host, int port) : conn_() {
        conn_ = R::connect(host, port);
    }

    void
    client::add(const std::string& name, tree& tree) {
        R::Cursor c = R::db_list().contains(name).run(*conn_);
        if (c.to_datum().extract_boolean()) {
            throw tree_exists_error("Tree " + name + "already exists");
        }
        // create a database based on the name of the tree
        R::db_create(name).run(*conn_);
        // each database has four tables:
        // "nodes," "subscriptions," "data," and "actions"
        R::db(name).table_create("nodes").run(*conn_);
        R::db(name).table_create("subscriptions").run(*conn_);
        R::db(name).table_create("actions").run(*conn_);
        R::db(name).table_create("log").run(*conn_);
    }

    void
    client::replace(const std::string& name, tree& tree) {
        R::Cursor c = R::db_list().contains(name).run(*conn_);
        // just drop/recreate the tables
        if (!c.to_datum().extract_boolean()) {
            add(name, tree);
            return;
        }
        R::db(name).table_drop("nodes").run(*conn_);
        R::db(name).table_drop("subscriptions").run(*conn_);
        R::db(name).table_drop("actions").run(*conn_);
        R::db(name).table_drop("log").run(*conn_);
        R::db(name).table_create("nodes").run(*conn_);
        R::db(name).table_create("subscriptions").run(*conn_);
        R::db(name).table_create("actions").run(*conn_);
        R::db(name).table_create("log").run(*conn_);

        R::db(name).table("nodes").changes();
    }

    void
    client::remove(const std::string& name) {
        R::Cursor c = R::db_list().contains(name).run(*conn_);
        if (!c.to_datum().extract_boolean()) { return; }
        R::db_drop(name).run(*conn_);
    }
}
