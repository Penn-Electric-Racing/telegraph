load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(name='com_github_grpc_grpc',
    sha256='1e3138fdd811015a66a22c4cafdc6ac4a117c5c70822008dc798f715d1239544',
    urls=['https://github.com/grpc/grpc/archive/v1.22.1.zip'],
    strip_prefix='grpc-1.22.1')

http_archive(name='json',
    sha256='541c34438fd54182e9cdc68dd20c898d766713ad6d901fb2c6e28ff1f1e7c10d',
    urls=['https://github.com/nlohmann/json/releases/download/v3.7.0/include.zip'],
    build_file='json.BUILD')

http_archive(name='hocon',
    sha256="b2cb3db5cfe02e8fb8a65cf89358912895b1a7efc852d83308af0811f221dbad",
    urls=['https://github.com/Penn-Electric-Racing/hocon/archive/8865ebbe434bfe586385514204d7c51b4825785a.zip'],
    strip_prefix='hocon-8865ebbe434bfe586385514204d7c51b4825785a')

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()

http_archive(
    sha256="41287ad7c4ea61b283d253460fc80e65738f8bd776a37586d28fb7a794d00d93",
    name = "com_grail_bazel_toolchain",
    strip_prefix = "bazel-toolchain-master",
    urls = ["https://github.com/grailbio/bazel-toolchain/archive/master.tar.gz"],
)

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain",
    llvm_version = "8.0.0",
)

load("@llvm_toolchain//:toolchains.bzl", "llvm_register_toolchains")

llvm_register_toolchains()
