load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")

def telegraph_deps_stage3():
    grpc_extra_deps()
