#ifndef __PER_CLIENT_HPP__
#define __PER_CLIENT_HPP__

#include <vector>
#include <string>
#include <memory>

#include <grpcpp/grpcpp.h>

#include "api.grpc.pb.h"
#include "api.pb.h"

namespace per {
    class tree;
    class node;

    // TOOD: Not finished,
    // some changes will have to be made to the context
    // calls to allow fetching of trees explicitly

    // the client has its own thread
    class client {
    public:
        client(const std::string& bind);
        ~client();

        // start the thread before connecting,
        // or the promises will never happen
        void start();

        void connect(const std::string& binding);
    private:
        grpc::ClientContext context_;
        std::shared_ptr<grpc::Channel> channel_;
        std::unique_ptr<proto::ContextManager::Stub> stub_;
    };
}
#endif
