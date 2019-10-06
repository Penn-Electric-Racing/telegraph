load(":rules.bzl", "stm32_library", "stm32_board", "stm32_platform")

# define all the platforms
# NOTE: They MUST be defined in this file, nowhere else!

stm32_platform("stm32f777vi", "platforms/startup_stm32f777xx.c", "platforms/stm32f777vi.lds")

# Add the stdlib library

stm32_library(name="stdlib",
           srcs=glob(["stdlib/**/*.c"]),
           hdrs=glob(["stdlib/**/*"]),
           platforms=["stm32f777vi"], # will create one target per platform
           visibility=["//visibility:public"],
           includes=["stdlib/libc", "stdlib/c++"], use_stdlib=False)

# Add the sdk library

stm32_library(name="sdk",
              srcs=glob(["sdk/**/*.cpp", "sdk/**/*.hpp"]),
              platforms=["stm32f777vi"],
              includes=["sdk"])

# Board targets can be uploaded
stm32_board(name="test", platform="stm32f777vi", srcs=["test.cpp"])
