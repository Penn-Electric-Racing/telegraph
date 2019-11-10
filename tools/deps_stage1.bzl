load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def telegraph_deps_stage1():
    http_archive(name='json',
        sha256='541c34438fd54182e9cdc68dd20c898d766713ad6d901fb2c6e28ff1f1e7c10d',
        urls=['https://github.com/nlohmann/json/releases/download/v3.7.0/include.zip'],
        build_file='@telegraph//:external/json.BUILD')

    http_archive(name='hocon',
        sha256="b2cb3db5cfe02e8fb8a65cf89358912895b1a7efc852d83308af0811f221dbad",
        urls=['https://github.com/Penn-Electric-Racing/hocon/archive/8865ebbe434bfe586385514204d7c51b4825785a.zip'],
        strip_prefix='hocon-8865ebbe434bfe586385514204d7c51b4825785a')

    http_archive(name='com_github_grpc_grpc',
                 sha256='e458d694273dd5f3b253b748771a0fa3ef1319aa2d59b65ac4938f51dda65434',
                 urls=['https://github.com/grpc/grpc/archive/ab7e5e8dec5ab3cda56fca9f9c25eb7cbeb2a017.zip'],
                 strip_prefix='grpc-ab7e5e8dec5ab3cda56fca9f9c25eb7cbeb2a017')

    http_archive(name="com_github_nanopb_nanopb",
                 sha256="2ed43c778e358c6f0142718dae968a6592e74e50fef1fe64f19342700fbf1049",
                 urls=["https://github.com/Penn-Electric-Racing/nanopb/archive/master.zip"],
                 strip_prefix="nanopb-master")
