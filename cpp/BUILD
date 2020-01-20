config_setting(name = 'is_linux', constraint_values = [ '@platforms//os:linux' ])
config_setting(name = 'is_windows', constraint_values = [ '@platforms//os:windows' ])
config_setting(name = 'is_macos', constraint_values = [ '@platforms//os:osx' ])

cpp17_opts = select({
    ':is_linux' : ['-std=c++17'],
    ':is_macos' : ['-std=c++17'],
    ':is_windows' : ['/std:c++17']
})

cc_library(name="telegraph",
   srcs=glob(["lib/**/*.hpp", "lib/**/*.cpp"]),
   includes=["proto", "lib"],
   copts=cpp17_opts,
   deps=['//:cc_proto_stream', '//:cc_proto_common', '//:cc_proto_api',
         '@json//:json', '@hocon//:hocon', '@boost//:beast', '@boost//:coroutine',
         '@boost//:asio', '@boost//:uuid', '@boost//:system']
)

cc_binary(name="device_test",
          srcs=glob(["main/device_test.cpp"]),
          copts=cpp17_opts,
          deps=[':telegraph'])


cc_binary(name="dummy_server",
          srcs=glob(["main/dummy_server.cpp"]),
          copts=cpp17_opts,
          deps=[":telegraph"])

cc_binary(name="dummy_client",
          srcs=glob(["main/dummy_client.cpp"]),
          copts=cpp17_opts,
          deps=[":telegraph"])

cc_binary(name="generate",
          srcs=glob(["main/generate.cpp"]),
          copts=cpp17_opts,
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
          deps=["//:cc_nanopb_stream", "//:cc_nanopb_log"],
          visibility=["//visibility:public"])

#cc_test(name="tree_test",
#        srcs=["test/tree-test.cpp"],
#        data=["test/example.conf"],
#        copts=["-std=c++17"],
#        deps=[":telegraph"])
