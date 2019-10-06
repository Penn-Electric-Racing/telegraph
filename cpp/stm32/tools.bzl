cross_copts = ['-nostdlib', '-target', 'arm-none-eabi', '-mcpu=cortex-m4', '-mfloat-abi=hard']
cross_lopts = ['-nostdlib', '-target', 'arm-none-eabi', '-mcpu=cortex-m4', '-mfloat-abi=hard']

def stm32_library(name, srcs=[], deps=[], includes=[]):
    native.cc_library(name=name,
                    srcs=srcs,
                    deps=deps + [":stm32_stdlib"],
                    includes=includes,
                    copts=cross_copts, nocopts='-fPIC',
                    linkopts=cross_lopts,
                    features=["-supports-pic", "-default_compile_flags", "-default_link_flags"])

def stm32_binary(name, srcs=[], deps=[], includes=[]):
    native.cc_binary(name=name,
                   srcs=srcs,
                   deps=deps + [":stm32_stdlib"],
                   includes=includes,
                   copts=cross_copts, nocopts='-fPIC',
                   linkopts=cross_lopts,
                   features=["-supports-pic", "-default_compile_flags", "-default_link_flags"])
