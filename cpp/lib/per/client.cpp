#include "client.hpp"
#include "errors.hpp"

#include <iostream>

#include <grpcpp/grpcpp.h>

#include <api.grpc.pb.h>
#include <api.pb.h>

using namespace libcom;

namespace per {
    client::client(const std::string& bind) : context_(), channel_(), stub_() {
        channel_ = grpc::CreateChannel(bind, grpc::InsecureChannelCredentials());
        stub_ = ContextManager::NewStub(channel_);
        // initiate the context deltas rpc
        Empty e;
    }
    client::~client() {
    }
}
