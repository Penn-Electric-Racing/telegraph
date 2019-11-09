load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

proto_library(
     name = "common_proto",
     srcs = ["common.proto"],
     visibility = ["//visibility:public"]
)

proto_library(
     name = "api_proto",
     srcs = ["api.proto"],
     deps = [":common_proto"],
     visibility = ["//visibility:public"]
)

proto_library(
     name = "log_proto",
     srcs = ["log.proto"],
     deps = [":common_proto"],
     visibility = ["//visibility:public"]
)

proto_library(
     name = "stream_proto",
     srcs = ["stream.proto"],
     deps = [":common_proto"],
     visibility = ["//visibility:public"]
)

cc_proto_library(name="cc_api_proto",
                 deps=["//proto:api_proto"],
                 visibility=["//visibility:public"])

cc_grpc_library(name="cc_api_grpc",
                srcs=["//proto:api_proto"], 
                deps=[":cc_api_proto"], grpc_only=True,
                visibility=["//visibility:public"])

cc_library(name="cc_api",
           deps=[":cc_api_proto", ":cc_api_grpc"],
           includes=["."],
           visibility=["//visibility:public"])
