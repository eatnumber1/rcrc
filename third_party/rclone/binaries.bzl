load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

_RCLONE_VERSION="v1.50.0"

def _add_rclone_build(platform, architecture, sha256=None):
  rule_name = "rclone_{platform}_{arch}".format(platform=platform, arch=architecture)
  if not native.existing_rule(rule_name):
    package_name = "rclone-{version}-{platform}-{arch}".format(version=_RCLONE_VERSION, platform=platform, arch=architecture)
    http_archive(
      name = rule_name,
      strip_prefix = package_name,
      urls = ["https://downloads.rclone.org/{version}/{package}.zip".format(version=_RCLONE_VERSION, package=package_name)],
      build_file_content = """exports_files(glob(["**"]))""",
      sha256 = sha256,
    )

def rclone_binaries():
  _add_rclone_build(
      platform = "linux",
      architecture = "amd64",
      sha256 = "3eb57540b5846f5812500243c37a34ef4bc90d608760e29f57cb82fef0a93f82",
  )
  _add_rclone_build(
      platform = "linux",
      architecture = "386",
      sha256 = "ba48abc1cfa528da347d66bef9a490a0e398dcd1c77fca13368c3a106f481176",
  )
  _add_rclone_build(
      platform = "osx",
      architecture = "amd64",
      sha256 = "15b399f923e7f16311263581cffdf9d49a1f1ff48ba8929637f2c555dc2541d3",
  )
  _add_rclone_build(
      platform = "osx",
      architecture = "386",
      sha256 = "f84dfeeabfd32aac4e8cbe0d230cd50ed70c2e8770770e429ed43f6f78aa52c0",
  )
