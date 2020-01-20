proto_library(
     name = "proto_common",
     srcs = ["common.proto"],
     visibility = ["//visibility:public"]
)

proto_library(
     name = "proto_api",
     srcs = ["api.proto"],
     deps = [":proto_common"],
     visibility = ["//visibility:public"]
)

proto_library(
     name = "proto_log",
     srcs = ["log.proto"],
     deps = [":proto_common"],
     visibility = ["//visibility:public"]
)

proto_library(
     name = "proto_stream",
     srcs = ["stream.proto"],
     deps = [":proto_common"],
     visibility = ["//visibility:public"]
)

cc_proto_library(name="cc_proto_common",
                 deps=["//:proto_common"],
                 visibility=["//visibility:public"])

cc_proto_library(name="cc_proto_api",
                 deps=["//:proto_api"],
                 visibility=["//visibility:public"])

cc_proto_library(name="cc_proto_stream",
                 deps=["//:proto_stream"],
                 visibility=["//visibility:public"])

load("@com_github_nanopb_nanopb//:gen_rules.bzl","cc_nanopb_library")

cc_nanopb_library(name="cc_nanopb_common", proto_library=":proto_common", base_name="common",
                  includes=["."], 
                  visibility=["//visibility:public"])

cc_nanopb_library(name="cc_nanopb_log", proto_library=":proto_log", base_name="log",
                  includes=["."], deps=[":cc_nanopb_common"],
                  visibility=["//visibility:public"])

cc_nanopb_library(name="cc_nanopb_stream", proto_library=":proto_stream", base_name="stream",
                  includes=["."], deps=[":cc_nanopb_common"],
                  visibility=["//visibility:public"])
