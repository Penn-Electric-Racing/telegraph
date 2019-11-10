load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

def telegraph_deps_stage2():
    # overload the boringssl to get rid of the warning
    #http_archive(name='boringssl',
    #    sha256='',
    #    urls=[''])

    grpc_deps()

