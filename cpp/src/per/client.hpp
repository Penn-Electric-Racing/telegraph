#ifndef __PER_CLIENT_HPP__
#define __PER_CLIENT_HPP__

#include <vector>
#include <string>
#include <memory>

#include <grpcpp/grpcpp.h>

#include <api.grpc.pb.h>
#include <api.pb.h>

namespace per {
    class tree;
    class node;
    class client {
    public:
        client(const std::string& bind);
        ~client();
    private:
        grpc::ClientContext context_;
        std::shared_ptr<grpc::Channel> channel_;
        std::unique_ptr<libcom::ContextManager::Stub> stub_;
    };
}
#endif
