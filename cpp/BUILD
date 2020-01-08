
cc_library(name="telegraph",
   srcs=glob(["lib/**/*.hpp", "lib/**/*.cpp"]),
   includes=["proto", "lib"],
   copts=["-std=c++17"],
   deps=['//proto:cc_api', '//proto:cc_stream',
         '@json//:json', '@hocon//:hocon', '@boost//:beast', '@boost//:coroutine',
         '@boost//:asio', '@boost//:uuid', '@boost//:system']
)

cc_binary(name="device_test",
          srcs=glob(["main/device_test.cpp"]),
          copts=["-std=c++17"],
          deps=[':telegraph'])


cc_binary(name="dummy_server",
          srcs=glob(["main/dummy_server.cpp"]),
          copts=["-std=c++17"],
          deps=[":telegraph"])

cc_binary(name="dummy_client",
          srcs=glob(["main/dummy_client.cpp"]),
          copts=["-std=c++17"],
          deps=[":telegraph"])

cc_binary(name="generate",
          srcs=glob(["main/generate.cpp"]),
          copts=["-std=c++17"],
          deps=[":telegraph"], visibility=["//visibility:public"])

#cc_binary(name="server",
#          srcs=glob(["main/server/**/*.hpp", "main/server/**/*.cpp"]),
#          copts=["-std=c++17"],
#          deps=[":telegraph"],
#          visibility=["//visibility:public"])

cc_library(name="generate_support",
          hdrs=glob(["gen/**/*.hpp"]),
          srcs=glob(["gen/**/*.cpp"]),
          includes=["gen"],
          deps=["//proto:cc_stream_nanopb", "//proto:cc_log_nanopb"],
          visibility=["//visibility:public"])

#cc_test(name="tree_test",
#        srcs=["test/tree-test.cpp"],
#        data=["test/example.conf"],
#        copts=["-std=c++17"],
#        deps=[":telegraph"])
