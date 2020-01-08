load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def telegraph_deps():
    http_archive(name='json',
        sha256='541c34438fd54182e9cdc68dd20c898d766713ad6d901fb2c6e28ff1f1e7c10d',
        urls=['https://github.com/nlohmann/json/releases/download/v3.7.0/include.zip'],
        build_file='@telegraph//:external/json.BUILD')

    http_archive(name='hocon',
        sha256="b2cb3db5cfe02e8fb8a65cf89358912895b1a7efc852d83308af0811f221dbad",
        urls=['https://github.com/Penn-Electric-Racing/hocon/archive/8865ebbe434bfe586385514204d7c51b4825785a.zip'],
        strip_prefix='hocon-8865ebbe434bfe586385514204d7c51b4825785a')

#    http_archive(name='websocketpp',
#        sha256='80f49469f1702ec64ef5d8e71302f6b781dca8ae06e560421cab42961c8c7ce6',
#        urls=['https://github.com/zaphoyd/websocketpp/archive/0.8.1.zip'],
#        strip_prefix='websocketpp-0.8.1',
#        build_file='@telegraph//:external/websocketpp.BUILD')

    http_archive(name="com_github_nanopb_nanopb",
                 sha256="c3f7f23836c7d2a12fcb6c7dadb42b1ba5201d9b4e67040c02b13462de6814b4",
                 urls=["https://github.com/Penn-Electric-Racing/nanopb/archive/master.zip"],
                 strip_prefix="nanopb-master")
    http_archive(
        name = "rules_cc",
        sha256 = "35f2fb4ea0b3e61ad64a369de284e4fbbdcdba71836a5555abb5e194cf119509",
        strip_prefix = "rules_cc-624b5d59dfb45672d4239422fa1e3de1822ee110",
        urls = [
                "https://mirror.bazel.build/github.com/bazelbuild/rules_cc/archive/624b5d59dfb45672d4239422fa1e3de1822ee110.tar.gz",
                "https://github.com/bazelbuild/rules_cc/archive/624b5d59dfb45672d4239422fa1e3de1822ee110.tar.gz",
                ],
    )
    http_archive(
        name = "com_google_protobuf",
        sha256="e4f8bedb19a93d0dccc359a126f51158282e0b24d92e0cad9c76a9699698268d",
        strip_prefix = "protobuf-3.11.2",
        urls=["https://github.com/protocolbuffers/protobuf/archive/v3.11.2.zip"]
    )
    git_repository(
        name = "com_github_nelhage_rules_boost",
        commit = "ed844db5990d21b75dc3553c057069f324b3916b",
        remote = "https://github.com/nelhage/rules_boost",
        shallow_since = "1576879360 -0800"
    )
