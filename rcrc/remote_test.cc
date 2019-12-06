#include "rcrc/rclone.h"

#include <iostream>

#include "gtest/gtest.h"
#include "nlohmann/json.hpp"
#include "rhutil/testing/assertions.h"
#include "rcrc/remote.h"

namespace rcrc {
namespace {

using json = ::nlohmann::json;

TEST(Remote, NoOp) {
  RClone::Options opts;
  //opts.remote.verbose = true;
  //opts.remote.url.SetURL("http://user:pass@localhost:1024");
  opts.config_file = "/dev/null";
  auto rclone_or = RClone::Create(std::move(opts));
  CHECK_OK(rclone_or.status());
  RClone rclone = std::move(rclone_or).ValueOrDie();

  //Remote::Options opts;
  //opts.url = "http://user:pass@localhost:5572";
  ////opts.verbose = true;
  //auto rc_or = Remote::Create(opts);
  //ASSERT_TRUE(IsOk(rc_or));
  //Remote rc = std::move(rc_or).ValueOrDie();
  Remote rc = rclone.GetRemote();

  ASSERT_TRUE(IsOk(rc.Call("rc/noop", {}).status()));

  //ASSERT_TRUE(IsOk(rc.OperationsList(
  //    {
  //      {"fs", "gdrive:"},
  //      {"remote", "/Torrents/passthepopcorn.me/UnknownPleasures"},
  //      {"opt", {
  //        {"showHash", true},
  //        {"recurse", true},
  //        {"filesOnly", true},
  //        {"noModTime", true}
  //      }}
  //    },
  //    [](json elem) -> rhutil::Status {
  //      std::cout << elem["Hashes"]["MD5"].get<std::string_view>() << " -> "
  //                << elem["Path"].get<std::string_view>() << std::endl;
  //      return rhutil::OkStatus();
  //    })));

  //Remote rc2(rc);
  //std::cerr << rc2.Call("core/memstats", {}).ValueOrDie() << std::endl;

  //Remote rc3 = rc.UnsharedCopy();
  //std::cerr << rc3.Call("operations/list", {
  //      {"fs", "gdrive:"},
  //      {"remote", "/Torrents/passthepopcorn.me/UnknownPleasures"},
  //      {"opt", {
  //        {"showHash", true},
  //        {"recurse", true},
  //        {"filesOnly", true},
  //        {"noModTime", true}
  //      }}
  //    }).ValueOrDie().dump(2) << std::endl;

  //std::cerr << rc3.Call("operations/list", {
  //      {"fs", "local:"},
  //      {"remote", "rcrc"},
  //      {"opt", {
  //        {"showHash", true},
  //        {"recurse", false},
  //        {"filesOnly", true},
  //        {"noModTime", true}
  //      }}
  //    }).ValueOrDie().dump(2) << std::endl;

  CHECK_OK(rclone.Shutdown());
}

}  // namespace
}  // namespace rcrc

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  CHECK_OK(rcrc::InitializeGlobals());
  return RUN_ALL_TESTS();
}
