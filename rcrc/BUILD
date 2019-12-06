package(default_visibility = ["//rcrc:internal"])

package_group(
    name = "internal",
    packages = ["//rcrc/..."],
)

cc_library(
    name = "rclone",
    srcs = ["rclone.cc"],
    hdrs = ["rclone.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":remote",
        "@curl//:curl",
        "@boost//:process",
        "@abseil//absl/random",
        "@abseil//absl/strings",
        "@abseil//absl/time",
    ],
    data = ["//third_party/rclone:rclone_bin"],
)

cc_library(
    name = "remote",
    srcs = ["remote.cc"],
    hdrs = ["remote.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@rhutil//rhutil:status",
        "@rhutil//rhutil/curl",
        "@curl//:curl",
        "@rhutil//rhutil/json",
        "@nlohmann_json//:json",
        "@abseil//absl/strings",
    ],
)

cc_test(
    name = "remote_test",
    srcs = ["remote_test.cc"],
    deps = [
        ":remote",
        ":rclone",
        "@nlohmann_json//:json",
        "@googletest//:gtest",
        "@rhutil//rhutil/testing:assertions",
    ],
)
