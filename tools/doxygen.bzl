# Adapted from https://jayconrod.com/posts/98/migrating-to-bazel--part-2

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar")

def _doxygen_archive_impl(ctx):
    """Generate a .tar.gz archive containing documentation using Doxygen.

    Args:
        name: label for the generated rule. The archive will be "%{name}.tar.gz".
        doxyfile: configuration file for Doxygen
        srcs: source files the documentation will be generated from.
    """
    doxyfile = ctx.file.doxyfile
    out_file = ctx.outputs.out

    ctx.actions.run_shell(
        inputs = ctx.files.srcs + ctx.files.mcss + [doxyfile],
        outputs = [out_file],
        command = "./external/m.css/documentation/doxygen.py '" + doxyfile.path + "'",
        env = {
            "TARBALL": out_file.path
        }
    )


doxygen_archive = rule(
    implementation = _doxygen_archive_impl,
    attrs = {
        "doxyfile": attr.label(
            mandatory = True,
            allow_single_file = True
        ),
        "srcs": attr.label_list(
            mandatory = True,
            allow_files = True
        ),
        "mcss": attr.label(
            default = "@m.css//:m_css",
            allow_files = True
        ),
    },
    outputs = {
        "out": "%{name}.tar.gz",
    },
)

def doxygen_deps():
    git_repository(
        name = "m.css",
        branch = "master",
        remote = "https://github.com/Penn-Electric-Racing/m.css.git",
    )
    # native.local_repository(
    #     name = "m.css",
    #     path = "../m.css",
    # )
