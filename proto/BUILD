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
