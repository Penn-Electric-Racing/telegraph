load(":tools.bzl", "stm32_library", "stm32_binary", "cross_copts")

cc_library(name="stm32_stdlib",
           hdrs=glob(["stdlib/**/*"]),
           includes=["stdlib"],
           copts=cross_copts)

stm32_library(name="sdk",
              srcs=glob(["sdk/**/*.cpp", "sdk/**/*.hpp"]),
              includes=["sdk"])

stm32_binary(name="test", srcs=["test.cpp"])
