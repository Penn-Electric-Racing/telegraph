#include "client.hpp"
#include "errors.hpp"

#include <iostream>

#include <grpcpp/grpcpp.h>

#include <api.grpc.pb.h>
#include <api.pb.h>

namespace per {
    client::client(const std::string& bind) : context_(), channel_(), stub_() {
        channel_ = grpc::CreateChannel(bind, grpc::InsecureChannelCredentials());
        stub_ = proto::ContextManager::NewStub(channel_);
        // initiate the context deltas rpc
        proto::Empty e;
    }
    client::~client() {
    }
}
