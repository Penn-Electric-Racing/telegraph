
def generate_telegraph_headers(name, input, config, header_name):
    native.genrule(name=name,
            srcs=[input],
            outs=[header_name],
            tools=["@telegraph//cpp:generate"],
            cmd="$(location @telegraph//cpp:generate) $< " + config + " $(location " + header_name + ")")


