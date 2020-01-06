workspace(name="telegraph")

# we can't put these in deps.bzl
# since load() cannot be used in a macro
load("//tools:deps.bzl", "telegraph_deps")
telegraph_deps()

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")
protobuf_deps()
