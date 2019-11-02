workspace(name="telegraph")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")


# we can't put these in deps.bzl
# since load() cannot be used in a macro
http_archive(name='com_github_grpc_grpc',
             sha256='e458d694273dd5f3b253b748771a0fa3ef1319aa2d59b65ac4938f51dda65434',
             urls=['https://github.com/grpc/grpc/archive/ab7e5e8dec5ab3cda56fca9f9c25eb7cbeb2a017.zip'],
             strip_prefix='grpc-ab7e5e8dec5ab3cda56fca9f9c25eb7cbeb2a017')

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()
load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
grpc_extra_deps()

load("//:deps.bzl", "telegraph_deps")
telegraph_deps()


