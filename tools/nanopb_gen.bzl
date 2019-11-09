load("@com_google_protobuf//:protobuf.bzl", "proto_gen")

def _cc_hdrs(srcs, use_grpc_plugin = False):
    ret = [s[:-len(".proto")] + ".pb.h" for s in srcs]
    if use_grpc_plugin:
        ret += [s[:-len(".proto")] + ".nanopb.h" for s in srcs]
    return ret

def _cc_srcs(srcs, use_grpc_plugin = False):
    ret = [s[:-len(".proto")] + ".pb.cc" for s in srcs]
    if use_grpc_plugin:
        ret += [s[:-len(".proto")] + ".nanopb.cc" for s in srcs]
    return ret


def cc_nanopb_library(name, srcs=[],
                      deps=[], cc_libs=[], visibility=[]):
    proto_gen(
        name=name + "_genproto",
        protoc="@com_google_protobuf",
        plugin="@com_github_nanopb_nanopb//:nanopb_plugin",
    )

    gen_srcs = _cc_srcs(srcs)
    gen_hdrs = _cc_hdrs(srcs)

    native.cc_library(
        name = name,
        srcs = gen_srcs,
        hdrs = gen_hdrs,
        deps = cc_libs + deps
    )
