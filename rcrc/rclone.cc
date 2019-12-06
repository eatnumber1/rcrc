#include "rcrc/rclone.h"

#include <string>
#include <optional>
#include <cstdint>
#include <cstdlib>
#include <system_error>
#include <algorithm>
#include <iterator>
#include <sched.h>

#include "rhutil/curl/curl.h"
#include "rhutil/errno.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "absl/random/random.h"
#include "absl/random/distributions.h"

namespace rcrc {

using ::rhutil::CurlCodeToStatus;
using ::rhutil::ErrnoAsStatus;
using ::rhutil::CurlStrDeleter;
using ::rhutil::ErrorCodeAsStatus;
using ::rhutil::CurlEasyGetInfo;
using ::rhutil::IsFailedPrecondition;
using ::rhutil::CurlEasyPerform;
using ::rhutil::CurlEasySetWriteCallback;
using ::rhutil::CurlEasySetopt;
using ::rhutil::CurlHandleDeleter;
using ::rhutil::CurlSListDeleter;
using ::rhutil::CurlShareSetopt;
using ::rhutil::CurlStrDeleter;
using ::rhutil::CurlURL;
using ::rhutil::HTTPCodeToStatus;
using ::rhutil::JSONParser;
using ::rhutil::InternalError;
using ::rhutil::InternalErrorBuilder;
using ::rhutil::InvalidArgumentErrorBuilder;
using ::rhutil::NewCurlSList;
using ::rhutil::OkStatus;
using ::rhutil::UnknownErrorBuilder;
using ::rhutil::Status;
using ::rhutil::StatusBuilder;
using ::rhutil::ProcessExitCodeToStatus;
using ::rhutil::StatusOr;
using ::rhutil::ThreadSafeCurlShare;
using Options = ::rcrc::RClone::Options;
using json = ::nlohmann::json;
using ParseEvent = ::rhutil::JSONParser::ParseEvent;
using CallbackAction = ::rhutil::JSONParser::CallbackAction;
namespace process = ::boost::process;

namespace {

std::string RandomString(absl::BitGen *gen, std::size_t len) {
  std::string result(len, '\0');
  std::generate(std::begin(result), std::end(result), [gen]() {
    constexpr std::string_view alphabet =
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_";
    return alphabet[absl::Uniform<size_t>(*gen, 0u, alphabet.size())];
  });
  return result;
}

}  // namespace

StatusOr<RClone> RClone::Create(Options opts) {
  Remote::Options ropts(std::move(opts.remote));
  CurlURL &url = ropts.url;

  ASSIGN_OR_RETURN(auto host, url.Get(CURLUPART_HOST, CURLU_URLDECODE));

  std::vector<std::string> args = { "rcd" };
  args.emplace_back("--quiet");
  if (!opts.config_file.empty()) {
    args.emplace_back(absl::StrCat("--config=", opts.config_file));
  }
  args.emplace_back(absl::StrCat("--rc-addr=", host.get(),
                                 ":", url.GetPort()));

  absl::BitGen gen;
  auto copy_or_generate =
    [&gen](std::unique_ptr<char, CurlStrDeleter> s) -> std::string {
      if (s != nullptr) return std::string(s.get());
      return RandomString(&gen, absl::Uniform<std::size_t>(gen, 16, 32));
    };

  std::string user = copy_or_generate(ropts.url.GetUser());
  args.emplace_back(absl::StrCat("--rc-user=", user));
  url.SetUser(user);

  std::string password = copy_or_generate(ropts.url.GetPassword());
  args.emplace_back(absl::StrCat("--rc-pass=", password));
  url.SetPassword(password);

  std::error_code err;
  auto child = std::make_unique<process::child>(
      "third_party/rclone/rclone", process::args(std::move(args)), err,
      process::std_out > stdout, process::std_err > stderr,
      process::std_in < process::null);
  RETURN_IF_ERROR(ErrorCodeAsStatus(err)) << "Couldn't start RClone";

  ASSIGN_OR_RETURN(Remote remote, Remote::Create(ropts));

  auto start_time = absl::Now();
  while (true) {
    RETURN_IF_ERROR(ErrnoAsStatus(sched_yield()));

    if (auto st = remote.RcNoopAuth({}).status(); st.ok()) {
      break;
    } else if (!IsFailedPrecondition(st)) {
      return std::move(st);
    }

    // RClone probably hasn't finished starting up yet, and we got a connection
    // failure. Try again later, unless we've reached our max attempts.
    constexpr auto kMaxWait = absl::Seconds(5);
    if (start_time + kMaxWait < absl::Now()) {
      return UnknownErrorBuilder() << "Failed to connect to RClone after "
                                   << kMaxWait;
    }
  }

  return RClone{std::move(child), std::move(remote)};
}

Remote RClone::GetRemote() {
  return remote_;
}

RClone::RClone(std::unique_ptr<boost::process::child> child, Remote remote)
  : child_(std::move(child)), remote_(std::move(remote))
{}

Status RClone::Shutdown() {
  RETURN_IF_ERROR(remote_.CoreQuit(EXIT_SUCCESS));
  std::error_code err;
  child_->wait(err);
  RETURN_IF_ERROR(ErrorCodeAsStatus(err));
  if (int ec = child_->exit_code(); ec != EXIT_SUCCESS) {
    return StatusBuilder(ProcessExitCodeToStatus(ec))
        << "RClone exited with unexpected error code";
  }
  return OkStatus();
}

RClone::~RClone() {
  if (child_ == nullptr) return;

  std::error_code err;
  bool running = child_->running(err);
  CHECK_OK(ErrorCodeAsStatus(err));

  if (running) CHECK_OK(Shutdown());
}

}  // namespace rcrc
