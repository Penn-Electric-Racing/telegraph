workspace(name="telegraph")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# we can't put these in deps.bzl
# since load() cannot be used in a macro
http_archive(name='com_github_grpc_grpc',
             sha256='1e3138fdd811015a66a22c4cafdc6ac4a117c5c70822008dc798f715d1239544',
             urls=['https://github.com/grpc/grpc/archive/v1.22.1.zip'],
             strip_prefix='grpc-1.22.1')

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()

load("//:deps.bzl", "telegraph_deps")
telegraph_deps()


