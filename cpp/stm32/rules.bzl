cross_copts = ['-nostdlib',
               '-target', 'arm-none-eabi', '-mcpu=cortex-m4', '-mfloat-abi=hard']
cross_lopts = ['-nostdlib', 
               '-target', 'arm-none-eabi', '-mcpu=cortex-m4', '-mfloat-abi=hard']

# define an stm32 platform (should only be run in the telegraph stm32 directory)
def stm32_platform(platform_name, startup_src, linker_src):
    stm32_library("startup", srcs=[startup_src], platforms=[platform_name], 
                  visibility=["//visibility:public"])

    native.exports_files(srcs=[linker_src], visibility=["//visibility:public"])

# define an stm32 library for a bunch of platforms (based on the configs above
# this defines one library per platform
def stm32_library(name, srcs=[], hdrs=[], 
                        deps=[], includes=[], 
                        platforms=[],
                        use_stdlib=True, visibility=[]):
    # add a dependency to the standard library if necessary
    all_deps = deps + ["@telegraph//cpp/stm32:stdlib"] if use_stdlib else deps

    for platform in platforms:
        # get platform-specific dependencies
        platform_deps = []
        for dep in all_deps:
            platform_deps.append(dep + '_'  + platform)

        platform_name = name + '_' + platform

        native.cc_library(name=platform_name,
                        srcs=srcs,
                        hdrs=hdrs,
                        deps=platform_deps,
                        includes=includes,
                        visibility=visibility,
                        copts=cross_copts,
                        nocopts="-fPIC",
                        linkopts=cross_lopts,
                        linkstatic=True,
                        features=["-supports-pic", "-default_compile_flags", "-default_link_flags"])

def stm32_executable(name, platform, srcs=[], deps=[], 
                       includes=[], use_stdlib=True, visibility=[]):
    # add a dependency to the standard library if necessary
    all_deps = deps + ["@telegraph//cpp/stm32:stdlib"] if use_stdlib else deps
    all_deps.append("@telegraph//cpp/stm32:startup")

    platform_deps = []
    for dep in all_deps:
        platform_deps.append(dep + '_'  + platform)

    ls_target = "@telegraph//cpp/stm32:platforms/" + platform + ".lds"
    platform_deps.append(ls_target)

    native.cc_binary(name=name,
               srcs=srcs, deps=platform_deps,
               includes=includes,
               visibility=visibility,
               nocopts="-fPIC",
               copts=cross_copts, 
               linkopts=cross_lopts + ["-T", "$(location " + ls_target + ")"],
               linkstatic=True,
               features=["-supports-pic", "-default_compile_flags", "-default_link_flags"])

# jlink upload rule
def jlink_upload_impl_(ctx):
    pass

jlink_upload = rule(
    implementation=jlink_upload_impl_,
    attrs={
        'binary': attr.label(mandatory=True)
    }
)

# defines a cc_executable for a board as well as a jlink upload rule for it

def stm32_board(name, platform, srcs=[], deps=[], includes=[],
                    visibility=[]):
    all_deps = deps + ["@telegraph//cpp/stm32:stm32_stdlib"]
    binary_name = name + "_binary"
    binary_label = ':' + name + '_binary'

    stm32_executable(binary_name, platform, 
                     srcs, deps, includes, True, visibility)

    jlink_upload(name=name,
                 binary=binary_label)
