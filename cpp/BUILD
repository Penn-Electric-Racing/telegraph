load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

cc_grpc_library(name="gen_grpc_api",
        srcs=["proto/api.proto"],
        deps=[])

cc_library(name="lib",
        srcs=glob(["lib/**/*.hpp", "lib/**/*.cpp"]),
        includes=["proto", "lib"],
        copts=["-std=c++17"],
        deps=[":gen_grpc_api", "@json//:lib", "@hocon//:lib"])

cc_binary(name="test",
          srcs=glob(["main/test.cpp"]),
          copts=["-std=c++17"],
          deps=[":lib"])
