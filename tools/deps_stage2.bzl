load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

def telegraph_deps_stage2():
    grpc_deps()

