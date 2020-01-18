load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

def telegraph_deps_extra():
    boost_deps()
    protobuf_deps()
