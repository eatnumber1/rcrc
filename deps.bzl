load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@rcrc//third_party/rclone:binaries.bzl", "rclone_binaries")

def rcrc_deps():
  rclone_binaries()

  if not native.existing_rule("googletest"):
    git_repository(
        name = "googletest",
        remote = "https://github.com/google/googletest.git",
        commit = "90a443f9c2437ca8a682a1ac625eba64e1d74a8a",
        shallow_since = "1565193450 -0400",
        repo_mapping = {"@com_google_absl": "@abseil"},
    )

  if not native.existing_rule("boringssl"):
    git_repository(
        name = "boringssl",
        commit = "7f9017dd3c60047d6fbc0f617d757c763af8867e",
        remote = "https://boringssl.googlesource.com/boringssl",
        shallow_since = "1542843106 +0000",
    )

  if not native.existing_rule("abseil"):
    http_archive(
        name = "abseil",
        sha256 = "0b62fc2d00c2b2bc3761a892a17ac3b8af3578bd28535d90b4c914b0a7460d4e",
        strip_prefix = "abseil-cpp-20190808",
        urls = ["https://github.com/abseil/abseil-cpp/archive/20190808.zip"],
    )

  if not native.existing_rule("rhutil"):
    #native.local_repository(
    #    name = "rhutil",
    #    path = "/Users/eatnumber1/Sources/rhutil",
    #)
    git_repository(
        name = "rhutil",
        remote = "https://github.com/eatnumber1/rhutil.git",
        commit = "2be4adf6679dc48eb58ca97b7f141bb077e69c5e",
        shallow_since = "1575606422 -0800",
    )

  if not native.existing_rule("curl"):
    http_archive(
        name = "curl",
        build_file = "@rhutil//third_party:curl.BUILD",
        sha256 = "d0393da38ac74ffac67313072d7fe75b1fa1010eb5987f63f349b024a36b7ffb",
        strip_prefix = "curl-7.66.0",
        urls = ["https://curl.haxx.se/download/curl-7.66.0.tar.gz"],
    )

  if not native.existing_rule("zlib"):
    http_archive(
        name = "zlib",
        sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
        strip_prefix = "zlib-1.2.11",
        build_file = "@rhutil//third_party:zlib.BUILD",
        urls = ["https://zlib.net/zlib-1.2.11.tar.gz"],
    )

  #if not native.existing_rule("com_google_protobuf"):
  #  http_archive(
  #      name = "com_google_protobuf",
  #      sha256 = "b4fdd8e3733cd88dbe71ebbf553d16e536ff0d5eb1fdba689b8fc7821f65878a",
  #      strip_prefix = "protobuf-3.9.1",
  #      urls = ["https://github.com/protocolbuffers/protobuf/releases/download/v3.9.1/protobuf-cpp-3.9.1.zip"],
  #  )

  if not native.existing_rule("rules_foreign_cc"):
    git_repository(
        name = "rules_foreign_cc",
        remote = "https://github.com/bazelbuild/rules_foreign_cc.git",
        commit = "6bb0536452eaca3bad20c21ba6e7968d2eda004d",
        shallow_since = "1571839594 +0200",
    )

  if not native.existing_rule("nlohmann_json"):
    http_archive(
        name = "nlohmann_json",
        strip_prefix = "include",
        sha256 = "541c34438fd54182e9cdc68dd20c898d766713ad6d901fb2c6e28ff1f1e7c10d",
        build_file = "@rhutil//third_party:json.BUILD",
        urls = ["https://github.com/nlohmann/json/releases/download/v3.7.0/include.zip"],
    )
