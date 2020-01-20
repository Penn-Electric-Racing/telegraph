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

