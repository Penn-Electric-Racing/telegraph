#ifndef __TELEGRAPH_FORWARDER_HPP__
#define __TELEGRAPH_FORWARDER_HPP__


namespace telegraph {
    class connection;
    class namespace_;

    // will take a connection and a local namespace
    // and forward incoming requests to that local namespace
    class forwarder {
    private:
        connection& conn_;
        namespace_* ns_;
    public:
        // will register handlers
        forwarder(connection& conn, namespace_* ns);

        // will unregister handlers in connection
        // and kill any ongoing operations
        ~forwarder();
    };
}

#endif
