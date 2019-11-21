
cc_library(name="telegraph",
        srcs=glob(["lib/**/*.hpp", "lib/**/*.cpp"]),
        includes=["proto", "lib"],
        copts=["-std=c++17"],
           deps=['//proto:cc_api', '@com_github_grpc_grpc//:grpc++',
                 '@json//:json', '@hocon//:hocon']
)

cc_binary(name="test",
          srcs=glob(["main/test.cpp"]),
          copts=["-std=c++17"],
          deps=[":telegraph"])

cc_binary(name="generate",
          srcs=glob(["main/generate.cpp"]),
          copts=["-std=c++17"],
          deps=[":telegraph"], visibility=["//visibility:public"])

cc_binary(name="server",
          srcs=glob(["main/server/**/*.hpp", "main/server/**/*.cpp"]),
          copts=["-std=c++17"],
          deps=[":telegraph"],
          visibility=["//visibility:public"])

cc_library(name="generate_support",
          hdrs=glob(["gen/**/*.hpp"]),
          srcs=glob(["gen/**/*.cpp"]),
          includes=["gen"],
          visibility=["//visibility:public"])

cc_test(name="tree_test",
        srcs=["test/tree-test.cpp"],
        data=["test/example.conf"],
        copts=["-std=c++17"],
        deps=[":telegraph"])
