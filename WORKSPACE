workspace(name="telegraph")

# we can't put these in deps.bzl
# since load() cannot be used in a macro
load("//tools:deps_stage1.bzl", "telegraph_deps_stage1")
telegraph_deps_stage1()
load("//tools:deps_stage2.bzl", "telegraph_deps_stage2")
telegraph_deps_stage2()
load("//tools:deps_stage3.bzl", "telegraph_deps_stage3")
telegraph_deps_stage3()
