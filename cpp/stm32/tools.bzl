
def stm32_library(name, srcs=[], deps=[]):
    native.cc_library(name=name,
                    srcs=srcs,
                    deps=deps,
                    copts=['-target', 'arm-none-eabi'])

def stm32_binary(name, srcs=[], deps=[]):
    native.cc_binary(name=name,
                   srcs=srcs,
                   deps=deps,
                   copts=['-target', 'arm-none-eabi']) 
