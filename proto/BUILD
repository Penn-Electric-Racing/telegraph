load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@com_github_nanopb_nanopb//:gen_rules.bzl","cc_nanopb_library")

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


# The standard c++ libraries generated from
# the above protobuffers

cc_proto_library(name="cc_api_proto",
                 deps=["//proto:api_proto"],
                 visibility=["//visibility:public"])

cc_proto_library(name="cc_stream_proto",
                 deps=["//proto:stream_proto"],
                 visibility=["//visibility:public"])

cc_grpc_library(name="cc_api_grpc",
                srcs=["//proto:api_proto"], 
                deps=[":cc_api_proto"], grpc_only=True,
                visibility=["//visibility:public"])

cc_library(name="cc_api",
           deps=[":cc_api_proto", ":cc_api_grpc"],
           includes=["."],
           visibility=["//visibility:public"])

cc_library(name="cc_stream",
           deps=[":cc_stream_proto"],
           includes=["."],
           visibility=["//visibility:public"])


# The nanopb generation system
# is separate from the main pb generation
# and has some odd things about it with respect
# to how it handles dependencies (not well!)

cc_nanopb_library(name="cc_common_nanopb", proto_library=":common_proto", base_name="common",
                  includes=["."], 
                  visibility=["//visibility:public"])

cc_nanopb_library(name="cc_log_nanopb", proto_library=":log_proto", base_name="log",
                  includes=["."], deps=[":cc_common_nanopb"],
                  visibility=["//visibility:public"])

cc_nanopb_library(name="cc_stream_nanopb", proto_library=":stream_proto", base_name="stream",
                  includes=["."], deps=[":cc_common_nanopb"],
                  visibility=["//visibility:public"])
