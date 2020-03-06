#ifndef __TELEGRAPH_FORWARDER_HPP__
#define __TELEGRAPH_FORWARDER_HPP__


namespace telegraph {
    class connection;
    class name_space;

    // will take a connection and a local namespace
    // and forward incoming requests to that local namespace
    class forwarder {
    private:
        connection& conn_;
        name_space* ns_;
    public:
        // will register handlers
        forwader(connection& conn, name_space* ns);

        // will unregister handlers in connection
        // and kill any ongoing operations
        ~forwarder();
    }
}

#endif
