config_setting(
    name = "linux_x86_64",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_64",
    ],
)

config_setting(
    name = "linux_x86_32",
    constraint_values = [
        "@platforms//os:linux",
        "@platforms//cpu:x86_32",
    ],
)

config_setting(
    name = "macos_x86_64",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_64",
    ],
)

config_setting(
    name = "macos_x86_32",
    constraint_values = [
        "@platforms//os:macos",
        "@platforms//cpu:x86_32",
    ],
)

genrule(
    name = "rclone_bin",
    visibility = ["//visibility:public"],
    srcs = select({
        ":linux_x86_64": ["@rclone_linux_amd64//:rclone"],
        ":linux_x86_32": ["@rclone_linux_386//:rclone"],
        ":macos_x86_64": ["@rclone_osx_amd64//:rclone"],
        ":macos_x86_32": ["@rclone_osx_386//:rclone"],
    }),
    outs = ["rclone"],
    cmd = """ cp "$(SRCS)" "$(OUTS)" """,
)
