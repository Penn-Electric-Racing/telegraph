workspace(name="telegraph")

# we can't put these in deps.bzl
# since load() cannot be used in a macro
load("//tools:deps.bzl", "telegraph_deps")
telegraph_deps()
load("//tools:deps_extra.bzl", "telegraph_deps_extra")
telegraph_deps_extra()
