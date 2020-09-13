# Adapted from https://jayconrod.com/posts/98/migrating-to-bazel--part-2

def _doxygen_archive_impl(ctx):
    """Generate a .tar.gz archive containing documentation using Doxygen.

    Args:
        name: label for the generated rule. The archive will be "%{name}.tar.gz".
        doxyfile: configuration file for Doxygen
        srcs: source files the documentation will be generated from.
    """
    doxyfile = ctx.file.doxyfile
    out_file = ctx.outputs.out
    out_dir_path = out_file.short_path[:-len(".tar.gz")]
    commands = [
        "mkdir -p %s" % out_dir_path,
        "out_dir_path=%s" % out_dir_path,
        "sed -e \"s:@@OUTPUT_DIRECTORY@@:$out_dir_path:\" <%s | doxygen -" % doxyfile.path,
        "tar czf %s -C %s ." % (out_file.path, out_dir_path),
    ]
    ctx.actions.run_shell(
        inputs = ctx.files.srcs + [doxyfile],
        outputs = [out_file],
        command = " && ".join(commands),
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
    },
    outputs = {
        "out": "%{name}.tar.gz",
    },
)
